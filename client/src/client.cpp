#include "client.hpp"
#include "settings.hpp"

Client::Client() {}

Client::~Client() {
	for (container_t::iterator it = connections.begin(); it != connections.end(); ++it) {
		it->closeConnection();
	}
}

int Client::newConnection(const std::string& server_ip, int port) {
	Connection conn;
	if (conn.initConnection(server_ip, port) == ERR) {
		return ERR;
	}
	connections.push_back(conn);
	return OK;
}

void Client::sendRequest(int index, const std::string& request) {
	connections[index].sendRequest(request);
}

std::string Client::receiveResponse(int index) {
	return connections[index].receiveResponse();
}
