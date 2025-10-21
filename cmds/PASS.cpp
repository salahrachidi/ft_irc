#include "../inc/Needs.hpp"
#include "../inc/Server.hpp"

std::string clientIp(int client_fd)
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char client_ip[INET_ADDRSTRLEN];

    if (getpeername(client_fd, (struct sockaddr *)&client_addr, &client_len) == -1)
		throw std::runtime_error("getpeername failed");
    if (inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip)) == NULL)
		throw std::runtime_error("inet_ntop failed");
    return std::string(client_ip);
}


void Server::handlePass(Client *client, const std::vector<std::string> &params) {
	std::string response;
	if (client->hasPassword()) {
		std::string msg = ERR_ALREADYREGISTRED(std::string("*"));
		send(client->getFd(),msg.c_str() , msg.size(), 0);
		return;
	}
	if (params.empty()) {
		response = ERR_NEEDMOREPARAMS(std::string("PASS"));
		sendReplay(client->getFd(), response);
		return;
	}
	if (params[0] != _password) {
		response = ERR_PASSWDMISMATCH(std::string("*"));
		sendReplay(client->getFd(), response);
		return;
	}
	client->setPassword(true);
}