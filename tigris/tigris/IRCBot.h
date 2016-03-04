#pragma once
#include <iostream>
#include <boost\asio.hpp>
#include <boost\algorithm\string.hpp>
#include <thread>
#include <fstream>
#include <algorithm>
//#include <ctime>

using namespace boost::asio;
using boost::asio::ip::tcp;



class IrcBot
{
public:
	IrcBot(boost::asio::io_service &ioService, tcp::resolver::iterator endpoint_it, const std::string &host, const std::string &port,
		const std::string &passMessage, const std::string &nickMessage, const std::string &userMessage, const std::string &joinMessage,
		const std::string &channelName, const std::string &serverWelcomeMessage);

	IrcBot(const IrcBot &other);

	void Start();
	void Stop();
	void RunCommand(const std::string &command);
	std::string GetHost();
	std::string GetPort();
	bool IsShuttingDown();
private:
	std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			elems.push_back(item);
		}
		return elems;
	}

	std::vector<std::string> split(const std::string &s, char delim) {
		std::vector<std::string> elems;
		split(s, delim, elems);
		return elems;
	}
	typedef void (IrcBot::*CommandHandler) (const std::string &);
	enum GameTypes { NONE, HANGMAN, WORDSCRAMBLE };

	void GameFunction_HANGMAN_InitWords();

	void ConsoleMessage(const std::string &message);

	void Command_Say(const std::string &command);
	void Command_QuickSay(const std::string &message);
	void Command_Join(const std::string &command);
	void Command_Custom(const std::string &command);
	void Command_Part(const std::string &command);
	void Command_Quit(const std::string &command);
	void Command_Exit(const std::string &command);


	void Send(const std::string &message);
	
	void Send_Say(const std::string &channel, const std::string message);void HandlePing(const std::string &message);

	void WriteHandler(const boost::system::error_code &error, std::size_t bytesWritten);
	
	void ReadHandler(const boost::system::error_code &error, std::size_t bytesRead);

	void ConnectHandler(const boost::system::error_code &error);

	void HandleRequest(const std::string &message);
	void HandleGameInput(const std::string &message);

	bool TryActivateGame(IrcBot::GameTypes game, const std::string &gameStartMessage);
	void Request_HangmanGame(const std::string &request);
	void Request_WordscrambleGame(const std::string &request);
	void Hangman_NextWord();
	void Wordscramble_NextWord();
	void GameHandler_HANGMAN(const std::string &message);
	void GameHandler_WORDSCRAMBLE(const std::string &message);
	void Hangman_IncorrectGuess(const std::string &message);
	void Request_StopGame(const std::string &request);

	std::string messageString;
	bool shuttingDown;
	std::string serverName;
	std::string serverWelcomeMessage;
	std::string pass;
	std::string nick;
	std::string user;
	std::string port;
	std::string join;
	std::string channel;
	tcp::socket sock;
	boost::asio::io_service &io;
	tcp::resolver::iterator endpoint_iterator;
	std::array<char, 4096> readBuffer;
	char wbuf[4096];
	std::string writeString;
	std::map<std::string, CommandHandler> commandMap;
	std::map<std::string, CommandHandler> requestMap;
	std::map<GameTypes, CommandHandler> gameHandlers;
	GameTypes activeGame;


	int hangmanCurrentWordIndex;
	int hangmanMaxGuesses;
	int hangmanCurrentGuesses;
	std::string hangmanCurrentWord;
	std::string hangmanCurrentGuessedWord;
	std::string hangmanUsedCharacters;
	std::vector<std::string> hangmanWords;
};





