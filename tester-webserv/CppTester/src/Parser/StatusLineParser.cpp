#include "StatusLineParser.hpp"
#include "ParserUtils.hpp"
#include "settings.hpp"

StatusLineParser::StatusLineParser()
: state(State::PARSING) {}

int StatusLineParser::parse(const std::string& buffer, std::size_t& index, Response& dest) {
	std::size_t pos = util::findEndLine(leftover, buffer, index);
	if (pos == std::string::npos) {
		return appendLeftover(buffer, index, buffer.size() - index);
	}
	appendLeftover(buffer, index, pos - index);
	util::skipEndLine(buffer, index);
	return parseStatusLine(dest);
}

int StatusLineParser::appendLeftover(const std::string& buffer, std::size_t& index, std::size_t len) {
	leftover.append(buffer, index, len);
	index += len;
	return OK;
}

int StatusLineParser::parseStatusLine(Response& dest) {
	static const ParseFunction parse_table[] = {
		&StatusLineParser::parseVersion,
		&StatusLineParser::parseSpace,
		&StatusLineParser::parseStatusCode,
		&StatusLineParser::parseSpace,
		&StatusLineParser::parseReasonPhrase,
		NULL
	};

	std::size_t index = 0;
	for (std::size_t i = 0; parse_table[i] != NULL; ++i) {
		if ((this->*(parse_table[i]))(dest, index) == ERR) {
			PRINT_ERR << "StatusLineParser Error: " << leftover << std::endl;
			return setError();
		}
	}
	leftover.clear();
	setState(State::COMPLETE);
	return OK;
}

int StatusLineParser::parseVersion(Response& dest, std::size_t& index) {
	if (leftover.compare(0, 5, "HTTP/") != 0) {
		return ERR;
	}
	index += 5;
	if (parseMajorVersion(dest, index) == ERR) {
		return ERR;
	} else if (parseChar(index, '.') == ERR) {
		return ERR;
	} else if (parseMinorVersion(dest, index) == ERR) {
		return ERR;
	}
	return OK;
}

int StatusLineParser::parseMajorVersion(Response& dest, std::size_t& index) {
	if (index >= leftover.size() || !isdigit(leftover[index])) {
		return ERR;
	}
	dest.major_version = leftover[index] - '0';
	index += 1;
	return OK;
}

int StatusLineParser::parseMinorVersion(Response& dest, std::size_t& index) {
	if (index >= leftover.size() || !isdigit(leftover[index])) {
		return ERR;
	}
	dest.minor_version = leftover[index] - '0';
	index += 1;
	return OK;
}

int StatusLineParser::parseChar(std::size_t& index, char c) {
	if (index >= leftover.size() || leftover[index] != c) {
		return ERR;
	}
	index += 1;
	return OK;
}

int StatusLineParser::parseSpace(Response& dest, std::size_t& index) {
	return parseChar(index, ' ');
}

int StatusLineParser::parseStatusCode(Response& dest, std::size_t& index) {
	std::size_t start = index;
	util::skip(leftover, index, isDigit);
	if (index - start != 3) {
		return ERR;
	}
	dest.status_code = std::stoi(&leftover[start]);
	return OK;
}

int StatusLineParser::parseReasonPhrase(Response& dest, std::size_t& index) {
	dest.reason_phrase = leftover.substr(index);
	index = leftover.size();
	return OK;
}

bool StatusLineParser::isComplete() const {
	return state == State::COMPLETE;
}

bool StatusLineParser::isError() const {
	return state == State::ERROR;
}

int StatusLineParser::setError() {
	setState(State::ERROR);
	return ERR;
}

void StatusLineParser::setState(State new_state) {
	state = new_state;
}

void StatusLineParser::reset() {
	setState(State::PARSING);
	leftover.clear();
}
