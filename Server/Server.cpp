#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <list>
#include <string>

#include "Session.hpp"

#pragma comment(lib,"Ws2_32.lib")

std::mutex sessionsMutex;
std::list<Session> sessions;


void Ipv4TcpServer()
{
	const auto s = socket(AF_INET, SOCK_STREAM, 0);

	if (s != INVALID_SOCKET)
	{

		const sockaddr_in serverAddress{
			/* ADDRESS_FAMILY sin_family    */ AF_INET,
			/* USHORT         sin_port;     */ htons(0x9090),
			/* IN_ADDR        sin_addr;     */ { /* INADDR_ANY */ },
			/* CHAR           sin_zero[8];  */ {},
		};

		if (bind(s, reinterpret_cast<const sockaddr*>(&serverAddress), sizeof serverAddress) == 0 and listen(s, SOMAXCONN) == 0)
		{

			std::cout
				<< "server: started" << std::endl
				<< "server port: " << ntohs(serverAddress.sin_port) << std::endl;

			for (;;)
			{
				const auto socket = accept(s, nullptr, nullptr);

				if (socket != INVALID_SOCKET)
				{
					Session session(socket, static_cast<int>(sessions.size()));

					std::cout
						<< "client connected IP:" << session.getClientIPAddress()
						<< std::endl;

					std::lock_guard<std::mutex> lock(sessionsMutex);
					sessions.push_back(std::move(session));
				}
			}
		}
		else
		{
			std::cerr << "server failed to start" << std::endl;
		}

		(void)closesocket(s);
	}
}


int main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
	{
		std::cerr << "Ws2_32.dll init failed" << std::endl;
	}

	std::thread sessionCleaner([&]
		{
			for (;;)
			{
				std::lock_guard<std::mutex> lock(sessionsMutex);
				(void)std::remove_if(sessions.begin(), sessions.end(), [](const Session& session) { return session.disconnected(); });
			}
		});
	sessionCleaner.detach();


	Ipv4TcpServer();


	WSACleanup();
}
