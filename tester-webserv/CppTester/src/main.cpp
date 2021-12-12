#include "Client/Client.hpp"
#include "settings.hpp"
#include "ResponseValidator/ResponseValidator.hpp"
#include "TestCases/TestCaseUtils/TestCaseUtil.hpp"
#include <bitset>
#include <iostream>

void printOptions() {
	Client::Settings::Flags options[] = {
		Client::Settings::NONE,
		Client::Settings::TIMEOUT,
		Client::Settings::PIPELINED,
	};

	for (auto option : options) {
		PRINT_INFO << std::bitset<16>(static_cast<short>(option)) << std::endl;
		// std::cout << std::bitset<32>((option)) << std::endl;
	}
}

Request::Pointer basicRequest(Address addr) {
	Request::Pointer r(new Request(addr));

	r->request_line = "GET / HTTP/1.1";
	r->header_fields["Host"] = addr.ip + ":" + std::to_string(addr.port);

	r->tag = "Tag";
	r->name = "Name";
	for (int i = 0; i < 10000; ++i) {
		r->message_body.append("a");
	}
	r->message_body = util::convertToChunked(r->message_body);
	// r->header_fields["Content-Length"] = std::to_string(r->message_body.size());
	r->header_fields["Transfer-Encoding"] = "chunked";
	return r;
}

ResponseValidator basicResponseValidator() {
	Response::Pointer response(new Response);

	response->message_body = "1234";
	response->status_code = 200;
	ResponseValidator val(response);
	return val;
}

int main() {
	Output::clearLog();

	Request::Pointer request = basicRequest(Address("localhost", 8080));
	// Request::Pointer request = basicRequest(Address("127.0.0.1", 8080));
	ResponseValidator validator = basicResponseValidator();
	// request->print();
	// validator.print();

	Client::Settings settings;
	settings.flags |= Client::Settings::PIPELINED;
	settings.pipeline_amount = 1;

	Client::RequestQueue q;
	q.push_back(Client::RequestPair(request, validator));
	q.push_back(Client::RequestPair(request, validator));
	Response::Pointer response(new Response);
	response->status_code = 404;
	ResponseValidator v2(response);
	q.push_back(Client::RequestPair(request, v2));
	Client::testRequests(q, q.front().first->server_addr, settings);
	return 0;
}
