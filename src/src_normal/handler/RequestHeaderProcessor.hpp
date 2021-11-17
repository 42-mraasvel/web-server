#pragma once

# include "handler/ConfigResolver.hpp"
# include "handler/RequestValidator.hpp"

class RequestHeaderProcessor
{
	public:
	private:
		ConfigResolver _config_resolver;
		RequestValidator _request_validator;
};
