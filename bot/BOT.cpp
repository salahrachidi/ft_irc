#include "../inc/Server.hpp"

//********************//
//        BOT         //
//********************//

void Server::checkChallengeCompletion(Challenge& challenge) {
    if (challenge.challenger_choice.empty() || challenge.target_choice.empty()) {
        return;
    }

    Channel* channel = _channelManager.search_for_channel(challenge.channel);
    if (!channel) return;

    std::string result;
    if (challenge.challenger_choice == challenge.target_choice) {
        result = "Draw! Both chose " + challenge.challenger_choice;
    } else if (
        (challenge.challenger_choice == "rock" && challenge.target_choice == "scissors") ||
        (challenge.challenger_choice == "paper" && challenge.target_choice == "rock") ||
        (challenge.challenger_choice == "scissors" && challenge.target_choice == "paper")
    ) {
        result = challenge.challenger + " wins! " + challenge.challenger_choice + 
                " beats " + challenge.target_choice;
    } else {
        result = challenge.target + " wins! " + challenge.target_choice + 
                " beats " + challenge.challenger_choice;
    }

    channel->broadcast(_NOTICE(channel->getName(), "RPS Result: " + result), "");
    activeChallenges.erase(challenge.challenger);
}

void Server::checkChallengeTimeouts() {
    time_t now = time(NULL);
    std::vector<std::string> to_remove;

    std::map<std::string, Challenge>::iterator it = activeChallenges.begin();
    for (; it != activeChallenges.end(); ++it) {
        const std::string& challenger = it->first;
        const Challenge& challenge = it->second;

        if (now - challenge.created_at > 120) {
            to_remove.push_back(challenger);
            Channel* channel = _channelManager.search_for_channel(challenge.channel);
            if (channel)
                channel->broadcast(_NOTICE(channel->getName(), "RPS challenge from " + challenger + " timed out"), "");
        }
    }

    std::vector<std::string>::iterator vIt = to_remove.begin();
    for (; vIt != to_remove.end(); vIt++) {
        activeChallenges.erase(*vIt);
    }
}

void Server::handleChoose(Client* client, const std::vector<std::string>& params) {
    checkChallengeTimeouts();
    if (params.size() < 2) {
        sendReplay(client->getFd(), _NOTICE(client->getNickName(), "Syntax: /choose <rock|paper|scissors> <channel>"));
        return;
    }
    
    std::string choice = params[0];
    std::string channelName = params[1];
    std::transform(choice.begin(), choice.end(), choice.begin(), ::tolower);

    if (choice != "rock" && choice != "paper" && choice != "scissors") {
        sendReplay(client->getFd(), _NOTICE(client->getNickName(), "Invalid choice. Valid options: rock, paper, scissors"));
        return;
    }

    bool found = false;
    std::map<std::string, Challenge>::iterator it = activeChallenges.begin();
    for (; it != activeChallenges.end(); it++) {
        if (it->second.channel != channelName) continue;
        if (client->getNickName() == it->second.challenger) {
            it->second.challenger_choice = choice;
            found = true;
			if (it->second.target_choice.empty()) {
                Client* target = findClientByNickname(it->second.target);
                if (target)
                    sendReplay(target->getFd(), _NOTICE(target->getNickName(), "[RPS] " + it->second.challenger + " has chosen. Your turn!"));
            }
        } else if (client->getNickName() == it->second.target) {
            if (!it->second.accepted) {
                sendReplay(client->getFd(), _NOTICE(client->getNickName(), "You must accept the challenge before choosing"));
                return;
            }
            it->second.target_choice = choice;
            found = true;
			if (it->second.challenger_choice.empty()) {

			Client* challenger = findClientByNickname(it->second.challenger);
		    if (challenger)
                sendReplay(challenger->getFd(), _NOTICE(challenger->getNickName(), it->second.target + " has chosen. Your turn!"));
			}
        }
        if (found) {
            checkChallengeCompletion(it->second);
            break;
        }
    }
    if (!found)
        sendReplay(client->getFd(), _NOTICE(client->getNickName(), "No active RPS challenge in " + channelName));
}

void Server::handleAccept(Client* client) {
    checkChallengeTimeouts();
    std::map<std::string, Challenge>::iterator it = activeChallenges.begin();
    for (; it != activeChallenges.end(); ++it) {
        if (it->second.target == client->getNickName()) {
            it->second.accepted = true;

            Channel * channel = _channelManager.search_for_channel(it->second.channel);
            if (channel)
                channel->broadcast(_NOTICE(channel->getName(), client->getNickName() + " accepted the RPS challenge!"), "");
            return;
        }
    }
    sendReplay(client->getFd(), _NOTICE(client->getNickName(), "No pending challenges to accept"));
}

void Server::handleRPS(Client* client, const std::vector<std::string>& params) {
    if (params.size() < 2) {
        sendReplay(client->getFd(), _NOTICE(client->getNickName(), " :Syntax: /rps <nickname> <channel>\r\n"));
        return;
    }

    std::string targetNick = params[0];
    std::string channelName = params[1];
    
    if (client->getNickName() == targetNick) {
        sendReplay(client->getFd(), _NOTICE(client->getNickName(), "You can't challenge yourself"));
        return;
    }
    Client* target = findClientByNickname(targetNick);
    if (!target) {
        sendReplay(client->getFd(), _NOTICE(client->getNickName(), "User " + targetNick + " not found"));
        return;
    }
    Channel* channel = _channelManager.search_for_channel(channelName);
    if (!channel) {
        sendReplay(client->getFd(), _NOTICE(client->getNickName(), "Channel " + channelName + " doesn't exist"));
        return;
    }

    Challenge newChallenge;
    newChallenge.challenger = client->getNickName();
    newChallenge.target = targetNick;
    newChallenge.challenger_choice = "";
    newChallenge.target_choice = "";
    newChallenge.accepted = false;
    newChallenge.created_at = time(NULL);
    newChallenge.channel = channelName;
        
    activeChallenges[client->getNickName()] = newChallenge;

    sendReplay(client->getFd(), _NOTICE(client->getNickName(), "Challenge sent to " + targetNick + " in " + channelName));
    sendReplay(target->getFd(), _NOTICE(target->getNickName(), "You've been challenged to RPS by " + client->getNickName() + " in " + channelName + ". Use /accept to confirm"));
    channel->broadcast(_NOTICE(channel->getName(), client->getNickName() + " challenged " + targetNick + " to RPS!"), "");
}
