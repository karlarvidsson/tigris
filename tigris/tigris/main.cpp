

#pragma once

#include "IRCBot.h"
#include <iostream>


using namespace std;



int main()
{
	cout << time(NULL) << endl;
	srand(time(NULL));
	
	std::locale swedish("swedish");
	std::locale::global(swedish);
	io_service ioservice;
	std::string host = "irc.twitch.tv";//"dreamhack.se.quakenet.org";
	std::string port = "6667"; //"6667";
	std::string pass = "PASS oauth:9rbuuqyql4zg19z0k9uas6pjzuvn0i\n"; //"PASS f\n";
	std::string nick = "NICK tigerlolbot\n"; //"NICK tigerlolbot\n";
	std::string user = "USER tigerlolbot\n";//"USER tigerloltv 0 * :Ronnie Reagan\n";
	std::string join = "JOIN #tigerloltv\n";//"JOIN #grapevine\r\n";
	std::string channel = "#tigerloltv"; //"#grapevine";
	std::string welcome = ":tmi.twitch.tv 001";

	tcp::resolver resolver(ioservice);
	tcp::resolver::query query(host, port);
	tcp::resolver::iterator endpoint_it = resolver.resolve(query);







	IrcBot ircBot(ioservice, endpoint_it, host, port, pass, nick, user, join, channel, welcome);

	ircBot.Start();
	std::thread t([&ioservice]() { srand(time(0)); ioservice.run(); });

	const int MAX_COMMAND_LENGTH = 256;
	char line[MAX_COMMAND_LENGTH];
	while (!ircBot.IsShuttingDown() && std::cin.getline(line, MAX_COMMAND_LENGTH))
	{
		ircBot.RunCommand(line);
	}
	if (!ircBot.IsShuttingDown())
	{
		std::cout << "Command length exceeded or unknown error. \r\nShutting down." << std::endl;
		ircBot.Stop();
	}
	t.join();
	system("pause");


	return 0;
}
