#include "Client.hpp"
#include "settings.hpp"
#include "ResponseValidator/ResponseValidator.hpp"
#include "Utility/utility.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <poll.h>

Client::Settings::Settings()
: timeout(DEFAULT_TIMEOUT), pipeline_amount(DEFAULT_PIPELINE), wait_close(DEFAULT_WAIT_CLOSE) {
	flags = TIMEOUT;
}

Client::Client(Settings settings)
: connfd(-1), settings(settings) {}

Client::~Client() {
	closeConnection();
}

void Client::testRequest(RequestPair request, Settings settings) {
	RequestQueue v;
	v.push_back(request);
	testRequests(v, request.first->server_addr, settings);
}

void Client::testRequests(const RequestQueue& requests, Address server_addr, Settings settings) {
	Client c(settings);
	if (!validSettings(settings) || !validRequests(requests)) {
		return;
	}
	if (c.initializeConnection(server_addr) == ERR) {
		return;
	}
	c.executeTransaction(requests);
	if (c.isError()) {
		// c.handleError(requests);
	}
}

bool Client::validSettings(const Settings& settings) {
	if (settings.timeout <= 0 || settings.pipeline_amount <= 0) {
		PRINT_ERR << "invalid settings" << std::endl;
		return false;
	}
	return true;
}

bool Client::validRequests(const RequestQueue& requests) {
	for (auto it = requests.begin(); it != requests.end(); ++it) {
		if (!validRequest(it->first)) {
			PRINT_ERR << "invalid input request" << std::endl;
			it->first->print();
			return false;
		}
	}
	return true;
}

bool Client::validRequest(const Request::Pointer request) {
	if (request->message_body.size() != 0) {
		return request->header_fields.contains("Content-Length") ||
			request->header_fields.contains("Transfer-Encoding");
	}
	return true;
}

int Client::initializeConnection(const Address& addr) {
	struct sockaddr_in server_addr;
	if (util::addressToSockaddr(addr, &server_addr) == ERR) {
		PRINT_ERR << "Could not resolve address: " << addr.ip << ":" << addr.port << std::endl;
		return ERR;
	}
	connfd = socket(AF_INET, SOCK_STREAM, 0);
	if (connfd == ERR) {
		return syscallError(_FUNC_ERR("socket"));
	}
	if (connectToServer(server_addr) == ERR) {
		closeConnection();
		return ERR;
	}
	return OK;
}

int Client::connectToServer(struct sockaddr_in server_addr) const {
	int retries = 0;
	while (connect(connfd, reinterpret_cast<const sockaddr*> (&server_addr), sizeof(server_addr)) == ERR) {
		if (retries == Client::CONNECT_MAX_RETRIES) {
			return ERR;
		}
		syscallError(_FUNC_ERR("Connect"));
		sleep(1);
		++retries;
	}
	return OK;
}

void Client::executeTransaction(RequestQueue requests) {
	prepareTransaction(requests);
	while (state == State::EXECUTING) {
		update(requests);
		if (isComplete()) {
			return;
		} else if (state == State::WAITING) {
			break;
		}
		int n = poll(&pfd, 1, POLL_TIMEOUT);
		if (n == ERR) {
			syscallError(_FUNC_ERR("poll"));
		} else if (shouldCloseConnection()) {
			warning("closing connection early due to poll");
			setError();
			closeConnection();
			return;
		} else if (n > 0) {
			executeEvents();
		}
	}
	waitForServer();
}

void Client::prepareTransaction(const RequestQueue& requests) {
	pfd.fd = connfd;
	pfd.events = POLLIN | POLLOUT;
	pfd.revents = 0;
	processing_request = requests.begin();
	state = State::EXECUTING;
	timer.reset();
}

/*
Update event
*/
void Client::update(RequestQueue& requests) {
	processResponses(requests);
	if (shouldGenerateRequestString(requests)) {
		if (settings.flags & Settings::PIPELINED) {
			generateMultipleRequests(requests);
		} else if (processing_request == requests.begin()) {
			generateSingleRequest();
		}
	}
	if (requests.empty()) {
		PRINT_INFO << "Client: [" << connfd << "]: processed all request/response pairs" << std::endl;
		if (settings.flags & Settings::WAIT_FOR_CLOSE) {
			setState(State::WAITING);
		} else {
			setState(State::COMPLETE);
		}
	} else if (shouldRemoveWriting(requests)) {
		if (processing_request == requests.end()) {
			PRINT_INFO << "Client: [" << connfd << "]: finished writing" << std::endl;
		}
		removeEvent(POLLOUT);
	} else if (string_generator.getRequestString().size() > 0) {
		addEvent(POLLOUT);
	}
	checkTimeout();
}

bool Client::shouldGenerateRequestString(const RequestQueue& requests) const {
	return processing_request != requests.end()
		&& string_generator.shouldGenerate(response);
}

void Client::generateMultipleRequests(const RequestQueue& requests) {
	std::size_t num = std::distance(requests.begin(), processing_request);
	std::size_t i = 0;
	while (i < settings.pipeline_amount - num && shouldGenerateRequestString(requests)) {
		string_generator.generate(processing_request->first);
		if (string_generator.isComplete()) {
			++processing_request;
			string_generator.resetState();
		}
		++i;
	}
}

void Client::generateSingleRequest() {
	string_generator.generate(processing_request->first);
	if (string_generator.isComplete()) {
		++processing_request;
		string_generator.resetState();
	}
}

void Client::processResponses(RequestQueue& requests) {
	while (!responses.empty()) {
		if (requests.size() == 0) {
			warning("no requests left to compare to" + std::to_string(responses.size()) + "remaining response vectors");
			return setError();
		}
		processResponse(responses.front(), requests.front().first, requests.front().second);
		requests.pop_front();
		responses.pop_front();
	}
}

void Client::processResponse(const ResponseVector responses, const Request::Pointer request,
							ResponseValidator validator) {
	if (!validator.isValidResponse(responses)) {
		validator.fail(*request, responses);
	} else {
		validator.pass(*request, responses);
	}
}

bool Client::shouldRemoveWriting(const RequestQueue& requests) const {
	return string_generator.getRequestString().size() == 0 && pfd.events & POLLOUT;
}

void Client::checkTimeout() {
	if (settings.flags & Settings::TIMEOUT && timer.elapsed() > static_cast<double>(settings.timeout)) {
		warning("timed out");
		setError();
	}
}

bool Client::shouldCloseConnection() {
	return pfd.revents & (POLLHUP | POLLERR | POLLNVAL);
}

void Client::executeEvents() {
	if (pfd.revents & POLLIN) {
		readEvent();
	}

	if (pfd.revents & POLLOUT) {
		writeEvent();
	}
}

/*
Read Event
*/
void Client::readEvent() {
	timer.reset();
	std::string buffer(BUFFER_SIZE, '\0');
	ssize_t n = recv(connfd, &buffer[0], BUFFER_SIZE, 0);
	if (n == ERR) {
		syscallError(_FUNC_ERR("recv"));
		return;
	} else if (n == 0) {
		warning("unexpected EOF read");
		// Server closed the connection
		setError();
		return;
	}
	buffer.resize(n);
	parseResponse(buffer);
}

void Client::parseResponse(const std::string& buffer) {
	std::size_t index = 0;
	while (index < buffer.size()) {
		if (response.size() == 0) {
			newResponsePointer();
		}
		if (response_parser.parse(buffer, index, *response.back()) == ERR) {
			warning("response parsing error");
			PRINT_DEBUG << buffer << std::endl;
			setError();
			return;
		} else if (response_parser.isComplete()) {
			PRINT_INFO << "Client: [" << connfd << "]: received response: [" << response.back()->status_code << "]" << std::endl;;
			finishResponse();
		}
	}
}

void Client::newResponsePointer() {
	response.push_back(Response::Pointer(new Response));
}

void Client::finishResponse() {
	if (isFinalResponse(response.back())) {
		responses.push_front(response);
		response.clear();
	} else {
		newResponsePointer();
	}
	response_parser.reset();
}

bool Client::isFinalResponse(Response::Pointer response) {
	return !(response->status_code >= 100 && response->status_code < 200);
	// return !((response->status_code >= 300 && response->status_code < 400)
	// 	|| (response->status_code >= 100 && response->status_code < 200));
}

/*
Write Event
*/
void Client::writeEvent() {
	timer.reset();
	const std::string& str = string_generator.getRequestString();
	ssize_t n = send(connfd, str.c_str(), std::min<std::size_t>(BUFFER_SIZE, str.size()), 0);
	if (n == ERR) {
		syscallError(_FUNC_ERR("send"));
		return;
	}
	PRINT_INFO << "Sent: " << n << " bytes" << std::endl;
	PRINT_DEBUG << str << std::endl;
	string_generator.eraseBytes(n);
}

void Client::waitForServer() {
	timer.reset();
	while (state == State::WAITING) {
		sleep(POLL_TIMEOUT);
		if (timer.elapsed() > settings.wait_close) {
			warning("timed out when waiting for server to close");
			setError();
			break;
		}
	}
	closeConnection();
}

void Client::handleError(const RequestQueue& requests) const {
	LOG_ERR << "Client: [" << connfd << "]: " << "error ocurred with " << requests.size() << " requests remaining" << std::endl;
	for (const Response::Pointer r : response) {
		LOG_INFO << "Received Response:" << std::endl;
		r->log();
	}
	ResponseVector empty;
	for (const RequestPair& it : requests) {
		it.second.fail(*it.first, empty);
	}
}

/*
Utility Functions
*/

void Client::closeConnection() {
	if (connfd == -1) {
		return;
	}

	util::closeFd(connfd);
	connfd = -1;
}

void Client::removeEvent(short int event) {
	pfd.events &= ~(event);
}

void Client::addEvent(short int event) {
	pfd.events |= event;
}

bool Client::isComplete() const {
	return state == State::COMPLETE;
}

bool Client::isError() const {
	return state == State::ERROR;
}

void Client::setError() {
	setState(State::ERROR);
}

void Client::setState(State new_state) {
	state = new_state;
}

void Client::warning(const std::string& x) const {
	PRINT_WARNING << "Client: [" << connfd << "]: " << x << std::endl;
}

void Client::printRevents() const {
	std::vector<std::string> result;

	if (pfd.revents & POLLIN) {
		result.push_back("POLLIN");
	}
	if (pfd.revents & POLLOUT) {
		result.push_back("POLLOUT");
	}
	if (pfd.revents & POLLERR) {
		result.push_back("POLLERR");
	}
	if (pfd.revents & POLLNVAL) {
		result.push_back("POLLNVAL");
	}
	if (pfd.revents & POLLHUP) {
		result.push_back("POLLHUP");
	}

	std::stringstream ss;
	for (std::size_t i = 0; i < result.size(); ++i) {
		if (i != 0) {
			ss << " | ";
		}
		ss << result[i];
	}
	if (ss.str().size() > 0) {
		PRINT_INFO << ss.str() << std::endl;
	}
}
