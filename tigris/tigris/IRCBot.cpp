#pragma once

#include "IRCBot.h"




IrcBot::IrcBot(boost::asio::io_service &ioService, tcp::resolver::iterator endpoint_it, const std::string &host, const std::string &port,
	const std::string &passMessage, const std::string &nickMessage, const std::string &userMessage, const std::string &joinMessage,
	const std::string &channelName, const std::string &serverWelcomeMessage)
	: io(ioService), sock(ioService), endpoint_iterator(endpoint_it), activeGame(NONE), shuttingDown(false), hangmanCurrentWord(""), hangmanMaxGuesses(6)
{
	serverName = host;
	pass = passMessage;
	nick = nickMessage;
	user = userMessage;
	this->port = port;
	join = joinMessage;
	channel = channelName;
	this->serverWelcomeMessage = serverWelcomeMessage;

	//srand(static_cast<unsigned int>(time(NULL)));



	GameFunction_HANGMAN_InitWords();
	//Wordscramble_NextWord();
	//std::cout << rand() % hangmanWords.size() << std::endl;
	//std::cout << rand() % hangmanWords.size() << std::endl;
	//std::cout << rand() % hangmanWords.size() << std::endl;
	//std::cout << rand() % hangmanWords.size() << std::endl;
	//std::cout << rand() % hangmanWords.size() << std::endl;
	//std::cout << rand() % hangmanWords.size() << std::endl;
	//std::cout << rand() % hangmanWords.size() << std::endl;

	// Command Map init
	commandMap.insert(std::pair<std::string, CommandHandler>("!say", &IrcBot::Command_Say));
	commandMap.insert(std::pair<std::string, CommandHandler>("!s", &IrcBot::Command_QuickSay));
	commandMap.insert(std::pair<std::string, CommandHandler>("!join", &IrcBot::Command_Join));
	commandMap.insert(std::pair<std::string, CommandHandler>("!part", &IrcBot::Command_Part));
	commandMap.insert(std::pair<std::string, CommandHandler>("!quit", &IrcBot::Command_Quit));
	commandMap.insert(std::pair<std::string, CommandHandler>("!exit", &IrcBot::Command_Exit));
	commandMap.insert(std::pair<std::string, CommandHandler>("!custom", &IrcBot::Command_Custom));

	// Request Map init
	requestMap.insert(std::pair<std::string, CommandHandler>("%hangman", &IrcBot::Request_HangmanGame));
	requestMap.insert(std::pair<std::string, CommandHandler>("%wordscramble", &IrcBot::Request_WordscrambleGame));
	requestMap.insert(std::pair<std::string, CommandHandler>("%stopgame", &IrcBot::Request_StopGame));

	// Game handler Map init
	gameHandlers.insert(std::pair<GameTypes, CommandHandler>(HANGMAN, &IrcBot::GameHandler_HANGMAN));
	gameHandlers.insert(std::pair<GameTypes, CommandHandler>(WORDSCRAMBLE, &IrcBot::GameHandler_WORDSCRAMBLE));


}
IrcBot::IrcBot(const IrcBot &other) : io(other.io), sock(other.io)//: io_Service(other.io_Service), sock(other.sock)
{
	serverName = other.serverName;
	pass = other.pass;
	nick = other.nick;
	user = other.user;
	port = other.port;
	join = other.join;
}

void IrcBot::Start()
{

	sock.async_connect(*endpoint_iterator, [this](boost::system::error_code ec)
	{
		ConnectHandler(ec);
	});
}
void IrcBot::Stop()
{
	shuttingDown = true;
	io.post([this]() { sock.close(); });
}
void IrcBot::RunCommand(const std::string &command)
{
	std::cout << "RunCommand: " << command << std::endl;

	std::vector<std::string> result = split(command, ' ');
	std::string tail = "";

	if (result.size() > 1)
		tail = command.substr(command.find_first_of(result[0]) + result[0].length() + 1);

	if (result.size() > 0)
	{
		if (commandMap.count(result[0]) > 0)
		{
			(this->*commandMap.at(result[0]))(tail);
		}
	}
}
std::string IrcBot::GetHost()
{
	return serverName;
}
std::string IrcBot::GetPort()
{
	return port;
}
bool IrcBot::IsShuttingDown()
{
	return shuttingDown;
}

void IrcBot::GameFunction_HANGMAN_InitWords()
{

	std::ifstream hangmanWordsFile("HANGMAN.txt");
	std::string line;
	while (std::getline(hangmanWordsFile, line))
	{
		hangmanWords.push_back(line);
	}
}

void IrcBot::ConsoleMessage(const std::string &message)
{
	std::cout << "CONSOLE: " << message << std::endl;
}

void IrcBot::Command_QuickSay(const std::string &message)
{
	if (message.length() <= 0)
	{	
		ConsoleMessage("No message input. Usage: !s [some message] (without brackets)");
		return;
	}
	std::string m = "PRIVMSG " + channel + " :" + message + "\r\n";
	ConsoleMessage("Sending: " + m);
	Send(m);
	//Sleep(500);
}

void IrcBot::Command_Say(const std::string &command)
{
	if (command.length() <= 0)
		return;
	std::string target = command.substr(0, command.find_first_of(' '));
	std::string message = command.substr(std::min<int>(command.find_first_of(target) + target.length() + 1, command.length()));
	if (message.length() <= 0)
	{
		ConsoleMessage("No message input. Usage: !say [#channel] [some message] (without brackets)");
		return;
	}
	std::string irc_message = "PRIVMSG " + target + " :" + message + "\r\n";
	ConsoleMessage("Sending: " + irc_message);
	Send(irc_message);
	//	Send("The current word: ____________________________________________");
	//Sleep(500);
}
void IrcBot::Command_Join(const std::string &command)
{
	if (command.length() <= 0)
		return;
	std::string target = command.substr(0, command.find_first_of(' '));
	if (target.length() <= 0)
	{
		ConsoleMessage("No channel input. Usage: !join [#channel] (without brackets)");
		return;
	}
	std::string irc_message = "JOIN " + target + "\r\n";
	ConsoleMessage("Joining " + target);
	Send(irc_message);
	//Sleep(500);
}
void IrcBot::Command_Custom(const std::string &command)
{
	if (command.length() <= 0)
		return;


	ConsoleMessage("Custom command: " + command);
	Send(command + "\r\n");
	//Sleep(500);
}
void IrcBot::Command_Part(const std::string &command)
{
	if (command.length() <= 0)
		return;
	std::string target = command.substr(0, command.find_first_of(' '));
	if (target.length() <= 0)
	{
		ConsoleMessage("No channel input. Usage: !part [#channel] (without brackets)");
		return;
	}
	std::string irc_message = "PART " + target + "\r\n";
	ConsoleMessage("Parting " + target);
	Send(irc_message);
	//Sleep(300);
}
void IrcBot::Command_Quit(const std::string &command)
{
	std::string message = command;
	std::string irc_message = "QUIT " + message + "\r\n";
	ConsoleMessage("Quitting with message: " + message);
	Send(irc_message);
	//Sleep(300);
}


void IrcBot::Command_Exit(const std::string &command)
{
	ConsoleMessage("Shutting down...");
	Command_Quit("ciao");
	Stop();
}

void IrcBot::Send(const std::string &message)
{
	//writeBuffer = message.data();
	for (int i = 0; i < message.size(); i++)
	{
		wbuf[i] = message[i];
	}
	try
	{
		async_write(sock, buffer(wbuf, message.size() * sizeof(char)), [this](boost::system::error_code ec, std::size_t bytesWritten) { WriteHandler(ec, bytesWritten); });
	}
	catch (boost::system::system_error writeError)
	{
		ConsoleMessage("Error encountered while sending/writing.");
	}
}
void IrcBot::Send_Say(const std::string &channel, const std::string message)
{
	if (channel.length() <= 0)
	{
		ConsoleMessage("No channel input. Usage: !say [#channel] [some message] (without brackets)");
		return;
	}
	if (message.length() <= 0)
	{
		ConsoleMessage("No message input. Usage: !say [#channel] [some message] (without brackets)");
		return;
	}
	std::string irc_message = "PRIVMSG " + channel + " :" + message + "\r\n";
	ConsoleMessage("Sending: " + irc_message);
	Send(irc_message);
}
void IrcBot::HandlePing(const std::string &message)
{
	int pingIndex = message.find("PING");
	if (pingIndex != std::string::npos) // && pingIndex == 0)
	{
		int len = 11;
		ConsoleMessage("sending pong");
		std::string p = "PONG " + message.substr(pingIndex + 5, len) + "\r\n";
		Send(p);
	}
}
void IrcBot::WriteHandler(const boost::system::error_code &error, std::size_t bytesWritten)
{
	// nothing here, we dont really care
}
void IrcBot::ReadHandler(const boost::system::error_code &error, std::size_t bytesRead)
{
	if (shuttingDown)
	{
		return;
	}
	if (!error)
	{
		messageString = readBuffer.data();

		std::cout << std::string(readBuffer.data(), bytesRead);
		HandlePing(messageString);

		// handle welcome message 001
		if (messageString.find(serverWelcomeMessage) == 0)//if (messageString.find(":" + serverName + " 001") == 0)
		{
			Send(join);
		}
		HandleRequest(messageString);

		if (activeGame != NONE)
		{
			HandleGameInput(messageString);
		}

		readBuffer.fill('\0');
		sock.async_read_some(buffer(readBuffer), [this](boost::system::error_code ec, std::size_t bytesRead) { ReadHandler(ec, bytesRead); });
	}
	else
	{
		ConsoleMessage(error.message());
	}
}

void IrcBot::ConnectHandler(const boost::system::error_code &error)
{
	ConsoleMessage("connect handler");
	if (!error)
	{
		boost::asio::write(sock, buffer(pass));
		boost::asio::write(sock, buffer(nick));
		boost::asio::write(sock, buffer(user));
		sock.async_read_some(buffer(readBuffer), [this](boost::system::error_code ec, std::size_t bytesRead) { ReadHandler(ec, bytesRead); });
	}
}

void IrcBot::HandleRequest(const std::string &message)
{
	if (message.find("PRIVMSG " + channel) != std::string::npos)
	{
		int messageStartIndex = message.find(channel) + channel.length() + 1;
		std::vector<std::string> result = split(message.substr(messageStartIndex + 1), ' ');
		result = split(result[0], '\r');

		if (result.size() > 0 && requestMap.count(result[0]) > 0)
		{
			(this->*requestMap.at(result[0]))(message.substr(messageStartIndex));
		}
	}
}
void IrcBot::HandleGameInput(const std::string &message)
{

	if (gameHandlers.count(activeGame) > 0)
		(this->*gameHandlers.at(activeGame))(message);

}

bool IrcBot::TryActivateGame(IrcBot::GameTypes game, const std::string &gameStartMessage)
{
	if (activeGame == NONE)
	{
		activeGame = game;
		Send_Say(channel, gameStartMessage);
		return true;
	}
	else
	{
		ConsoleMessage("Request for start game while game already active.");
		Send_Say(channel, "A game is already active. Use %stopgame to stop the active game.");
		Sleep(500);
		return false;
	}
}
void IrcBot::Request_HangmanGame(const std::string &request)
{
	ConsoleMessage("HangmanGame START: ");

	if (hangmanWords.empty())
	{
		ConsoleMessage("HangmanGame - No words loaded, cant start game. ");
		Send_Say(channel, "Hangman game load failure. Cannot start.");
		return;
	}

	if (IrcBot::TryActivateGame(HANGMAN, "Hangman game starting! How to play: guess letters by typing single letters. Guess complete word by %word [guess] (without brackets)."))
	{
		Hangman_NextWord();

	}
}
void IrcBot::Request_WordscrambleGame(const std::string &request)
{
	ConsoleMessage("WordscrambleGame START: ");

	if (hangmanWords.empty())
	{
		ConsoleMessage("WordscrambleGame - No words loaded, cant start game. ");
		Send_Say(channel, "Wordscramble game load failure. Cannot start.");
		return;
	}

	if (IrcBot::TryActivateGame(WORDSCRAMBLE, "Wordscramble game starting! How to play: guess what the scrambled word is by typing the correct word."))
	{
		Wordscramble_NextWord();

	}
}
void IrcBot::Hangman_NextWord()
{
	hangmanCurrentWordIndex = rand() % hangmanWords.size();
	hangmanCurrentWord = hangmanWords[hangmanCurrentWordIndex];
	hangmanCurrentGuessedWord = "";
	hangmanUsedCharacters = "";
	hangmanCurrentGuesses = 0;

	for (std::string::iterator it = hangmanCurrentWord.begin(); it != hangmanCurrentWord.end(); ++it)
	{
		if (*it != ' ')
			hangmanCurrentGuessedWord += "_ ";
		else
			hangmanCurrentGuessedWord += "  ";
	}

	ConsoleMessage("hangmangame current word: " + hangmanCurrentWord);
	Send_Say(channel, "The current word: " + hangmanCurrentGuessedWord);
	Sleep(500);
	//	Command_Say(channel + " Incorrect characters: " + hangmanUsedCharacters);
}
void IrcBot::Wordscramble_NextWord()
{
	int r = rand();
	hangmanCurrentWordIndex = r % hangmanWords.size();
	hangmanCurrentWord = hangmanWords[hangmanCurrentWordIndex];
	std::transform(hangmanCurrentWord.begin(), hangmanCurrentWord.end(), hangmanCurrentWord.begin(), ::tolower);
	hangmanCurrentGuessedWord = hangmanCurrentWord;
	//std::transform(hangmanCurrentGuessedWord.begin(), hangmanCurrentGuessedWord.end(), hangmanCurrentGuessedWord.begin(), ::tolower);
	std::random_shuffle(hangmanCurrentGuessedWord.begin(), hangmanCurrentGuessedWord.end());


	
	ConsoleMessage("wordscramblegame current word: " + hangmanCurrentWord);
	Send_Say(channel, hangmanCurrentGuessedWord);
	Sleep(500);
}
void IrcBot::GameHandler_HANGMAN(const std::string &message)
{
	//	ConsoleMessage("hangman handler yao");

	if (message.find("PRIVMSG " + channel) != std::string::npos)
	{
		int messageStartIndex = message.find(channel) + channel.length() + 1;


		std::string reply = message.substr(messageStartIndex);

		if (reply.size() - 3 == 1) // the message is exactly one letter in size and we assume it is from someone playing the game, so we count it
		{
			char c = message[messageStartIndex + 1];

			if (((int)c >= 65 && (int)c <= 90) || ((int)c >= 97 && (int)c <= 122)) // the chosen letter is in permitted ascii range, so we go with it
			{
				std::cout << "CONSOLE: " << "Chosen char: " << c << " (int: " << (int)c << ")" << std::endl;
				//hangmanUsedCharacters += c;

				char lower_c = c;
				tolower(lower_c);
				std::string correctWord_lower = hangmanCurrentWord;
				std::string guessedWord_lower = hangmanCurrentGuessedWord;
				boost::algorithm::to_lower(guessedWord_lower);
				boost::algorithm::to_lower(correctWord_lower);


				if (correctWord_lower.find(lower_c) != std::string::npos && guessedWord_lower.find(lower_c) == std::string::npos)
				{
					// the letter is correct and not already chosen
					for (int i = 0; i < correctWord_lower.length(); i++)
					{
						if (correctWord_lower[i] == lower_c)
							hangmanCurrentGuessedWord.replace(i * 2, 1, 1, hangmanCurrentWord[i]);
					}
					ConsoleMessage("Letter correct, inserted into current guessed word");
					Send_Say(channel, "The current word: " + hangmanCurrentGuessedWord);
					//	Command_Say(channel + " Incorrect characters: " + hangmanUsedCharacters);

					if (hangmanCurrentGuessedWord.find('_') == std::string::npos)
					{
						std::string winner = message.substr(1, message.find_first_of("!") - 1);
						ConsoleMessage("All letters found (last found by " + winner + ")");
						Send_Say(channel, "Last letter found by " + winner + "!");
						Hangman_NextWord();
					}
				}
				else if (guessedWord_lower.find(lower_c) == std::string::npos && hangmanUsedCharacters.find(lower_c) == std::string::npos)
				{
					// the letter is not already chosen, and it is incorrect
					hangmanUsedCharacters += lower_c;
					hangmanCurrentGuesses++;
					Hangman_IncorrectGuess(message);
				}
				else
					ConsoleMessage("Chosen character has been chosen previously, ignoring.");
			}
			else
			{
				ConsoleMessage("Chosen char not in permitted ascii range, ignoring.");
			}
		}
		else if (reply.find("%word") != std::string::npos) // found "%word" command: someone wants to give it a go at guessing the entire word
		{
			int start = reply.find_first_of("%word") + 6;
			int end = (reply.length() - 2) - start;
			std::string answer = reply.substr(start, end);
			ConsoleMessage("Guessed word: " + answer);

			boost::algorithm::to_lower(answer);
			std::string correctWord = hangmanCurrentWord;
			boost::algorithm::to_lower(correctWord);

			if (answer == correctWord)
			{
				std::string winner = message.substr(1, message.find_first_of("!") - 1);
				ConsoleMessage("Correct word by " + winner + "!");
				Send_Say(channel, "Correct word by " + winner + "!");
				Hangman_NextWord();
			}
			else
			{
				// the word guess was incorrect
				hangmanCurrentGuesses++;
				ConsoleMessage("Word guess incorrect");
				Send_Say(channel, "The word guess \"" + answer + "\" is incorrect");
				Hangman_IncorrectGuess(message);
			}

		}

	}
}

void IrcBot::GameHandler_WORDSCRAMBLE(const std::string &message)
{
	if (message.find("PRIVMSG " + channel) != std::string::npos)
	{
		int messageStartIndex = message.find(channel) + channel.length() + 1;


		std::string reply = message.substr(messageStartIndex);
		std::string m = reply.substr(1, reply.find("\r\n") - 1);
		if (m.find(" ") == std::string::npos)
		{
			// cant find space, so this is a single word to be processed
			std::transform(m.begin(), m.end(), m.begin(), ::tolower);
			if (m.compare(hangmanCurrentWord) == 0)
			{
				std::string winner = message.substr(1, message.find_first_of("!") - 1);
				ConsoleMessage("Right word guessed by " + winner);
				Send_Say(channel, winner + " wins!");
				Wordscramble_NextWord();
			}
		}
	}

}

void IrcBot::Hangman_IncorrectGuess(const std::string &message)
{
	std::stringstream ss;
	ss << " Incorrect characters: " << hangmanUsedCharacters << " (";
	ss << hangmanCurrentGuesses << "/" << hangmanMaxGuesses << ")";
	Send_Say(channel, ss.str());
	ConsoleMessage("Incorrect character chosen " + ss.str());
	Sleep(500);

	if (hangmanCurrentGuesses >= hangmanMaxGuesses)
	{
		std::string loser = message.substr(1, message.find_first_of("!") - 1);
		ConsoleMessage("The game is lost (lost by " + loser + ")");
		Send_Say(channel, "The poor dude is hanging from the gallows, courtesy of " + loser + "!");
		Hangman_NextWord();
	}
}
void IrcBot::Request_StopGame(const std::string &request)
{
	if (activeGame != NONE)
	{
		activeGame = NONE;

		Send_Say(channel, "Current game stopped on request.");
	}
}