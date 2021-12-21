#include "ResponseUpdator.hpp"
#include "settings.hpp"
#include "outputstream/Output.hpp"
#include "utility/status_codes.hpp"
#include "utility/utility.hpp"
#include "fd/FdTable.hpp"
#include "Response.hpp"
#include "CgiHandler.hpp"
#include "FileHandler.hpp"
#include <iostream>

ResponseUpdator::ResponseUpdator() {}

void	ResponseUpdator::update(FdTable & fd_table, Response & response)
{
	if (!response.isFinished())
	{
		updateHandler(response);
	}
	else if (response.status == Response::COMPLETE)
	{
		return;
	}

	if (response.status == Response::SPECIAL && !StatusCode::isStatusCodeNoMessageBody(response.status_code))
	{
		setSpecialMessageBody(fd_table, response);
	}

	if (response.handler->isComplete())
	{
		response.markComplete(response.handler->getStatusCode());
	}
	_generator.generateString(response);
}

void	ResponseUpdator::updateHandler(Response & response)
{
	try
	{
		response.handler->update(response.message_body);
	}
	catch (std::exception const & e)
	{
		PRINT_ERR << "ResponseUpdator: caught update exception: " << e.what() << std::endl;
		response.handler->exceptionEvent();
	}
	if (response.handler->isError())
	{
		response.markSpecial(response.handler->getStatusCode());
	}
}

void	ResponseUpdator::setSpecialMessageBody(FdTable & fd_table, Response & response)
{
	if (response.config_info.result == ConfigInfo::REDIRECT
		&& response.status_code == response.config_info.resolved_location->_return.first)
	{
		processRedirectResponse(response);
		response.status = Response::COMPLETE;
	}
	else if (response.config_info.result == ConfigInfo::AUTO_INDEX_ON
			&& response.status_code == StatusCode::STATUS_OK)
	{
		processAutoIndex(response);
		response.status = Response::COMPLETE;
	}
	if (response.message_body.empty())
	{
		if (response.status_code == StatusCode::REQUEST_TIMEOUT
			|| response.error_page_attempted
			|| !isErrorPageRedirected(fd_table, response))
		{
			setOtherErrorPage(response);
			response.status = Response::COMPLETE;
		}
	}
}

void	ResponseUpdator::processRedirectResponse(Response & response)
{
	std::string	redirect_text = response.config_info.resolved_location->_return.second;
	if (StatusCode::isStatusCode3xx(response.status_code))
	{
		response.effective_request_uri = redirect_text;
		response.message_body = "Redirect to " + redirect_text + "\n";
	}
	else
	{
		response.message_body = redirect_text;
	}
}

void	ResponseUpdator::processAutoIndex(Response & response)
{
	if (WebservUtility::list_directory(response.config_info.resolved_target, response.config_info.resolved_file_path, response.message_body) == ERR)
	{
		response.message_body.erase();
		response.markSpecial(StatusCode::INTERNAL_SERVER_ERROR);
	}
}

bool	ResponseUpdator::isErrorPageRedirected(FdTable & fd_table, Response & response)
{
	std::string file_path;
	ConfigResolver	error_page_resolver(response.config_info.resolved_server);
	if (error_page_resolver.resolveErrorPage(response.status_code) == OK)
	{
		if (response.file_handler.redirectErrorPage(fd_table, error_page_resolver.getConfigInfo().resolved_file_path, response.status_code) == OK)
		{
			response.resetErrorPageRedirection();
			return true;
		}
	}
	return false;
}

void	ResponseUpdator::setOtherErrorPage(Response & response)
{
	response.message_body = WebservUtility::itoa(response.status_code) + " "
					+ StatusCode::getStatusMessage(response.status_code) + "\n";
}
