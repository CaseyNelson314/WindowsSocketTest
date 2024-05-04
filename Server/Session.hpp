#pragma once

#include <future>
#include <WinSock2.h>

class Session
{
	SOCKET socket;
	int id;
	std::future<int> feature;

	int session()
	{
		for (;;)
		{
			char buffer[1024];
			const auto bytes = recv(socket, buffer, sizeof buffer, 0);
			if (bytes == 0)
			{
				std::cout << "client disconnected" << std::endl;
				return 0;
			}
			else if (bytes == SOCKET_ERROR)
			{
				std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
				//return WSAGetLastError();
			}
			else
			{
				std::cout << "client" << id << ":";
				std::cout.write(buffer, bytes) << std::endl;
			}
		}
	}
public:

	Session(SOCKET socket, int id)
		: socket(socket)
		, id(id)
		, feature(std::async(std::launch::async, &Session::session, this))
	{
	}

	Session(Session&& session) noexcept
		: socket(session.socket)
		, id(session.id)
		, feature(std::move(session.feature))
	{
		session.socket = INVALID_SOCKET;
	}

	~Session()
	{
		if (socket != INVALID_SOCKET)
		{
			(void)closesocket(socket);
		}
	}

	Session& operator=(Session&& session) noexcept
	{
		socket = session.socket;
		feature = std::move(session.feature);
		return *this;
	}

	bool disconnected() const
	{
		return feature.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
	}

	int getId() const
	{
		return id;
	}	

	std::string getClientIPAddress() const
	{
		sockaddr_in clientAddress;
		int addressLength = sizeof clientAddress;
		getpeername(socket, reinterpret_cast<sockaddr*>(&clientAddress), &addressLength);
		CHAR address[256];
		inet_ntop(clientAddress.sin_family, reinterpret_cast<void*>(&clientAddress.sin_addr), address, sizeof address);
		return address;
	}

};
