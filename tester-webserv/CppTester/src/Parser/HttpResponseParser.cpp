#include "HttpResponseParser.hpp"
#include "settings.hpp"
#include <algorithm>
#include <limits>

static bool isValidResponseHeader(std::string const &key,
								 std::string const &value, HeaderField const &header) {
	return true;
}

HttpResponseParser::HttpResponseParser()
: state(State::PARSE_STATUS_LINE), header_field_parser(isValidResponseHeader, MAX_HEADER_SIZE) {}

int HttpResponseParser::parse(const std::string& buffer, std::size_t& index, Response& dest) {
	while (index < buffer.size()) {
		switch (state) {
			case State::PARSE_STATUS_LINE:
				parseStatusLine(buffer, index, dest);
				break;
			case State::PARSE_HEADER:
				parseHeader(buffer, index, dest);
				break;
			case State::PARSE_CONTENT:
				parseContent(buffer, index, dest);
				break;
			case State::PARSE_CHUNKED:
				parseChunked(buffer, index, dest);
				break;
			case State::ERROR:
				return ERR;
			case State::COMPLETE:
				return OK;
		}
	}
	return OK;
}

void HttpResponseParser::parseStatusLine(const std::string& buffer,
		std::size_t & index, Response & response) {
	if (status_line_parser.parse(buffer, index, response) == ERR) {
		setError();
	} else if (status_line_parser.isComplete()) {
		setState(State::PARSE_HEADER);
	}
}

void HttpResponseParser::parseHeader(const std::string& buffer,
		std::size_t & index, Response & response) {
	if (header_field_parser.parse(buffer, index) == ERR) {
		PRINT_ERR << "HeaderFieldParser Error" << std::endl;
		setError();
	} else if (header_field_parser.isComplete()) {
		response.header_fields.swap(header_field_parser.getHeaderField());
		processHeader(response.header_fields);
	}
}

void HttpResponseParser::processHeader(const HeaderField& header) {
	checkContent(header);
}

void HttpResponseParser::checkContent(const HeaderField& header) {
	if (header.contains("Content-Length")) {
		checkContentLength(header.get("Content-Length").first->second);
	} else if (header.contains("Transfer-Encoding")) {
		checkTransferEncoding(header.get("Transfer-Encoding").first->second);
	} else {
		setState(State::COMPLETE);
	}
}

void HttpResponseParser::checkContentLength(const std::string& value) {
	for (char c : value) {
		if (!isdigit(c)) {
			return setError();
		}
	}
	std::size_t length;
	if (util::strtoul(value, length) == -1) {
		return setError();
	} else if (length == 0) {
		return setState(State::COMPLETE);
	}
	content_parser.setContentLength(length);
	setState(State::PARSE_CONTENT);
}

void HttpResponseParser::checkTransferEncoding(const std::string& value) {
	if (!util::caseInsensitiveEqual(value, "chunked")) {
		return setError();
	}
	setState(State::PARSE_CHUNKED);
}


void HttpResponseParser::parseContent(const std::string& buffer,
		std::size_t & index, Response & response) {
	if (content_parser.parse(buffer, index) == ERR) {
		PRINT_ERR << "ContentParser Error" << std::endl;
		setError();
	} else if (content_parser.isComplete()) {
		response.message_body.swap(content_parser.getContent());
		setState(State::COMPLETE);
	}
}

void HttpResponseParser::parseChunked(const std::string& buffer,
		std::size_t & index, Response & response) {
	if (chunked_content_parser.parse(buffer, index) == ERR) {
		PRINT_ERR << "ChunkedParser Error" << std::endl;
		setError();
	} else if (chunked_content_parser.isComplete()) {
		chunked_content_parser.appendMessageBody(response.message_body);
		chunked_content_parser.appendHeader(response.header_fields);
		setState(State::COMPLETE);
	}
}

bool HttpResponseParser::isError() const {
	return state == State::ERROR;
}

bool HttpResponseParser::isComplete() const {
	return state == State::COMPLETE;
}

void HttpResponseParser::setError() {
	setState(State::ERROR);
}

void HttpResponseParser::setState(State new_state) {
	state = new_state;
}

void HttpResponseParser::reset() {
	state = State::PARSE_STATUS_LINE;
	status_line_parser.reset();
	header_field_parser.reset();
	content_parser.reset();
	chunked_content_parser.reset();
}
