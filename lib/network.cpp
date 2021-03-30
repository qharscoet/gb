#include "network.h"

/* Mosly c/c'ed code from MSDN Winsock tutorial
	 need to cleanup and make it compilable on linux */

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "ws2_32.lib")
	#define close(socket) closesocket(socket)
	#define SHUT_RDWR SD_BOTH
#else
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <errno.h>

	typedef int SOCKET;
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define DEFAULT_PORT "27015"


SOCKET other_socket = INVALID_SOCKET;
enum network_state state = NOT_CONNECTED;

int init_network()
{
	// No need to init WSADATA or equivalanet on unix systems
	#ifdef _WIN32
		WSADATA wsaData;
		int iResult;

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
		{
			printf("WSAStartup failed: %d\n", iResult);
			return 1;
		}

		return iResult;
	#else
		return 1;
	#endif
}

void cleanup()
{
	#ifdef _WIN32
		WSACleanup();
	#endif
}

int get_error(){
#ifdef _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}

uint64_t init_listen_socket(const char *port)
{
	if(port == NULL) port = DEFAULT_PORT;

	int iResult;
	struct addrinfo *result = NULL, *ptr = NULL, hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, port, &hints, &result);

	if (iResult != 0)
	{
		printf("getaddrinfo failed: %d\n", iResult);
		cleanup();
		return 1;
	}

	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %d\n", get_error());
		freeaddrinfo(result);
		cleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", get_error());
		freeaddrinfo(result);
		close(ListenSocket);
		cleanup();
		return 1;
	}

	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("Listen failed with error: %d\n", get_error());
		close(ListenSocket);
		cleanup();
		return 1;
	}

	SOCKET ClientSocket;
	ClientSocket = INVALID_SOCKET;

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET)
	{
		printf("accept failed: %d\n", get_error());
		close(ListenSocket);
		cleanup();
		return 1;
	}

	other_socket = ClientSocket;
	printf("SERVER CONNECTED\n");
	state = CONNECTED_SERVER;

	return ClientSocket;
}

int init_connect_socket(const char* addr, const char* port)
{
	if(addr == NULL) addr = "127.0.0.1";
	if(port == NULL) port = DEFAULT_PORT;

	printf("addr %s port %s\n", addr,port);

	int iResult;
	struct addrinfo *result = NULL,
					*ptr = NULL,
					hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(addr, port, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed: %d\n", iResult);
		cleanup();
		return 1;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
						   ptr->ai_protocol);

	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %d\n", get_error());
		freeaddrinfo(result);
		cleanup();
		return 1;
	}

	// Connect to server.
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		close(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	// Should really try the next address returned by getaddrinfo
	// if the connect call failed
	// But for this simple example we just free the resources
	// returned by getaddrinfo and print an error message

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server!\n");
		cleanup();
		return 1;
	}

	other_socket = ConnectSocket;

	printf("CLIENT CONNECTED\n");
	state = CONNECTED_CLIENT;

	return iResult;
}

int send_data(const char* buf, int length, uint8_t blocking)
{
	// sending data
	// in: SOCKET sender
	fd_set set;
	FD_ZERO(&set);
	FD_SET(other_socket, &set);
	timeval timeout = {0, 0};
	int sel = select(other_socket + 1, NULL, &set, NULL, blocking?NULL:&timeout);
	if (FD_ISSET(other_socket, &set))
	{
		return send(other_socket, buf, length, 0);
	}
	return 0;
}

int receive_data(char* buf, int length, uint8_t blocking)
{
	fd_set set;
	FD_ZERO(&set);
	FD_SET(other_socket, &set);
	timeval timeout = {0, 0};
	int sel = select(other_socket + 1, &set, NULL, NULL, blocking?NULL:&timeout);
	if (FD_ISSET(other_socket, &set))
	{
		int recv_bytes = recv(other_socket, buf, length, 0);
		if(!recv_bytes)
		{
			close_socket();
		}

		return recv_bytes;
	}

	return 0;
}

int close_socket()
{
	if(other_socket != INVALID_SOCKET)
	{
		int iResult = shutdown(other_socket, SHUT_RDWR);
		if (iResult == SOCKET_ERROR)
		{
			printf("shutdown failed with error: %d\n", get_error());
			close(other_socket);
			cleanup();
			state = NOT_CONNECTED;
			return 1;
		}

		// cleanup
		close(other_socket);
	}
	cleanup();
	state = NOT_CONNECTED;

	return 0;
}

enum network_state get_network_state()
{
	return state;
}