#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>

#pragma comment(lib,"Ws2_32.lib")

void Ipv4TcpClient()
{
	const auto s = socket(
		/* _In_ int af           */ AF_INET,
		/* _In_ int type         */ SOCK_STREAM,
		/* _In_ int protocol     */ 0);

	if (s != INVALID_SOCKET)
	{
		sockaddr_in clientAddress{
			/* ADDRESS_FAMILY sin_family    */ AF_INET,
			/* USHORT         sin_port;     */ htons(0x9090),
			/* IN_ADDR        sin_addr;     */ {},
			/* CHAR           sin_zero[8];  */ {},
		};
		clientAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

		if (connect(s, reinterpret_cast<const sockaddr*>(&clientAddress), sizeof clientAddress) == 0)
		{
			CHAR address[256];
			inet_ntop(clientAddress.sin_family, reinterpret_cast<void*>(&clientAddress.sin_addr), address, sizeof address);
			std::cout
				<< "client: started " << std::endl
				<< "server port: " << ntohs(clientAddress.sin_port) << std::endl
				<< "server address: " << address << std::endl;

			for (;;)
			{
				std::string string;
				std::cin >> string;
				send(s, string.c_str(), string.length(), 0);
			}

		}
		else
		{
			std::cerr << "server not found" << std::endl;
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

	Ipv4TcpClient();

	WSACleanup();
}
