// CardGameServerRemake.cpp : Defines the entry point for the console application.
//

#include "Mouse.h"
#include "stdafx.h"
#include "iostream"
#include "string"
#include "WS2tcpip.h"
#include "list"
#include "fstream"
#include "Card.h"
#include "sstream"
#include "vector"
#include "algorithm"
#include "conio.h"
#include "thread"
#include "mutex"
#include "deque"
#include <regex>

#pragma comment (lib,"ws2_32.lib")

int cardwidth = 100;
int cardheight = 150;
int lastupdate = 0;

std::list<Card*> Cards = std::list<Card*>();
std::list<Card*> lastselected = std::list<Card*>();

int players = 5;

std::list<std::string> playerid = std::list<std::string>();


std::mutex mtx;


int returnrandom(int before, int now) {



	if (before == now) {


		now = returnrandom(before, rand() & 2 + 0);

	}

	return now;
}


void shakelist(std::list<Card*>* lastselected) {

	int thebefore = 0;

	std::list<Card*> korten2 = std::list<Card*>();

	std::list<Card*> korten3 = std::list<Card*>();



	for (std::list<Card*>::iterator it = lastselected->begin(); it != lastselected->end(); it++) {

		Card* korta = *it;

		thebefore = returnrandom(thebefore, thebefore);

		if (thebefore == 0) {
			korten2.push_back(korta);
		}
		else {
			korten3.push_back(korta);
		}
		korta->flipped = true;
		OutputDebugStringW(std::to_wstring(thebefore).c_str());
		OutputDebugString(L"\n");
		korta->xcord = 20;
		korta->ycord = 20;



	}

	for (std::list<Card*>::iterator it = korten2.begin(); it != korten2.end();it++) {
		lastselected->remove(*it);
		thebefore = returnrandom(thebefore, thebefore);

		if (thebefore == 0) {
			lastselected->push_front(*it);
		}
		else {
			lastselected->push_back(*it);
		}

	}

	for (std::list<Card*>::iterator it = korten3.begin(); it != korten3.end();it++) {
		lastselected->remove(*it);
		thebefore = returnrandom(thebefore, thebefore);

		if (thebefore == 0) {
			lastselected->push_front(*it);
		}
		else {
			lastselected->push_back(*it);
		}

	}


}



Card* getbyid(int cardid) {
	try {
		for (std::list<Card*>::iterator it = lastselected.begin(); it != lastselected.end(); it++) {
			Card* korta = *it;
			if (korta->id == cardid) {
				return korta;
			}
		}
	}
	catch (std::exception e) {
		return NULL;
	}
	return NULL;
}


int server = 0;





void sendtoport() {

	try {
		players = players + 1;
		WSADATA wsData;
		WORD ver = MAKEWORD(2, 2);



		int wsOk = WSAStartup(ver, &wsData);


		if (wsOk != 0) {

			std::cout << "Error wsOk!" << std::endl;
			sendtoport();


		}

		SOCKET listening = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


		if (listening == INVALID_SOCKET) {
			std::cout << "Invalid socket!" << std::endl;
		}

		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(54000);
		hint.sin_addr.S_un.S_addr = INADDR_ANY;



		bind(listening, (sockaddr*)&hint, sizeof(hint));




		listen(listening, SOMAXCONN);

		sockaddr_in client;
		int clientSize = sizeof(client);




		SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
		const char optval = 1;
		int resulter = setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY || SO_KEEPALIVE, &optval, sizeof(int));



		char host[NI_MAXHOST]; // remote name
		char service[NI_MAXHOST]; // service i.e port that the client connection is through


		ZeroMemory(host, NI_MAXHOST); //ZeroMemory
		ZeroMemory(service, NI_MAXHOST); //ZeroMemory

		if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
			std::cout << host << " connected on port " << service << std::endl;
		}
		else {
			inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
			std::cout << host << " connected on port " << ntohs(client.sin_port) << std::endl;
		}

		closesocket(listening);

		std::thread* nytttrad = new std::thread(sendtoport);
		
			char buf[4096];
		
		while (true) {
			//mtx.lock();
			ZeroMemory(buf, 4096);

			// Wait for client to send data :)

			// Echo message back to client


			int bytesReceived = recv(clientSocket, buf, 4096, 0);
			std::string message = std::string(buf, 0, bytesReceived);
			if (bytesReceived == SOCKET_ERROR) {
				std::cout << "Error socket error!" << std::endl;
				break;

			}
			if (bytesReceived == 0) {
				std::cout << "Client disconnected " << std::endl;
				break;
			}


			ZeroMemory(buf, 4096);
			if (message == "CREATE ID") {
				mtx.lock();
				std::string userInput = std::to_string(players);
				std::cout << userInput;
				int sendResult = send(clientSocket, userInput.c_str(), userInput.size() + 1, 0);	
				if (sendResult == SOCKET_ERROR) {
					std::cout << "Oh no! \n";
				}

				mtx.unlock();

			}


			if (std::regex_search(message, std::regex("CARDUPDATETOSERVER ")) > 0) {
				std::stringstream ss(message);
				std::string thecardid;
				std::string theflipped;
				std::string thexcord;
				std::string theycord;
				ss >> thecardid;
				ss >> thecardid;
				ss >> theflipped;
				ss >> thexcord;
				ss >> theycord;
				mtx.lock();
				Card* whichisit = getbyid(stoi(thecardid));
				whichisit->xcord = stoi(thexcord);
				whichisit->ycord = stoi(theycord);
				whichisit->flipped = stoi(theflipped);
				lastselected.remove(whichisit);
				lastselected.push_back(whichisit);
				mtx.unlock();
			}


			if (message == "CARD WIDTH") {
				mtx.lock();
				std::string userInput = std::to_string(cardwidth);

				int sendResult = send(clientSocket, userInput.c_str(), userInput.size() + 1, 0);
				if (sendResult == SOCKET_ERROR) {
					std::cout << "Oh no! \n";
				}
				mtx.unlock();

			}
			if (message == "CARD HEIGHT") {
				std::string userInput = std::to_string(cardheight);

				int sendResult = send(clientSocket, userInput.c_str(), userInput.size() + 1, 0);
				if (sendResult == SOCKET_ERROR) {
					std::cout << "Oh no! \n";
				}

			}

			if (message == "CARD GET") {
				mtx.lock();
				for (std::list<Card*>::iterator it = lastselected.begin(); it != lastselected.end();it++) {
					Card* korta = *it;
					std::string userInputer = korta->texture + " | " + std::to_string(korta->id) + " | " + std::to_string(korta->flipped) + " | " + std::to_string(korta->xcord) + " | " + std::to_string(korta->ycord);
					send(clientSocket, userInputer.c_str(), userInputer.size() + 1, 0);
					ZeroMemory(buf, 4096);
					recv(clientSocket, buf, 4096, 0);
				}
				std::string closeCommand = "CLOSE";

				send(clientSocket, closeCommand.c_str(), closeCommand.size() + 1, 0);

				mtx.unlock();

			}
			
	

			if (message == "CARD UPDATE SIMPEL") {
				std::string letsbuildit = "";
				mtx.lock();
				for (std::list<Card*>::iterator it = lastselected.begin(); it != lastselected.end(); it++) {
					Card* thecardtoupdate = *it;
					letsbuildit = letsbuildit + std::to_string(thecardtoupdate->id) + " " + std::to_string(thecardtoupdate->flipped) + " " + std::to_string(thecardtoupdate->xcord) + " " + std::to_string(thecardtoupdate->ycord) + " \n";
				}
				mtx.unlock();
				send(clientSocket, letsbuildit.c_str(), letsbuildit.size() + 1, 0);
			}

			if (message == "CARD FLIP") {
				mtx.lock();
				shakelist(&lastselected);
				lastupdate = lastupdate + 1;
				std::cout << "CARD FLIP COMMAND!";
				mtx.unlock();
			}


			ZeroMemory(buf, 4096);

		}

		closesocket(clientSocket);

	}
	catch (std::exception e) {
		std::cout << "Error!";
	}

	//WSACleanup();

}






int main()
{


	std::ifstream input("card/cards.txt");

	std::string line;

	int at = 0;	

	for (std::string line; getline(input, line); )
	{

		LPCWSTR sd;
		std::wstring input = std::wstring(line.begin(), line.end());


		Card* kortet = new Card(at, 20, 20, line, true);

	
		lastselected.push_back(kortet);
		Cards.push_back(kortet);



		//std::cout << "Added card! " << std::to_string(at) << " " << line << std::endl;



		at = at + 1;


	}




	std::thread* nytttrad = new std::thread(sendtoport);

	while (true) {
		int b;
		std::cin >> b;
	}

	return 0;
}

