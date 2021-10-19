#include "Response.hpp"
#include "fd/File.hpp"

Response::Response(): file(NULL), status(NOT_COMPLETE), status_code(0) {}
