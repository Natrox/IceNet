/*
	Copyright (c) 2013-2014 Sam Hardeman

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#pragma once

#include <WinSock2.h>
#include <Ws2tcpip.h>

#include <vector>
#include "ClientContainer.h"

/*
	ICECAT Networking		v2.0

	class NetworkControl:

	This class is the main interface for all networking activities (Internally).
*/

#define BACKLOG 8

namespace IceNet
{
	typedef unsigned short CLIENT_ID; 

	// Prototypes
	class Client;
	class ClientProxy;
	class Packet;

	class NetworkControl
	{
	public:
		// TCP is always on, UDP is enabled by default
		enum SV_PROTOCOL
		{
			PACK_TCP,
			PACK_UDP
		};

		// Error codes in case things fail
		enum SV_ERRORCODE
		{
			ERROR_NONE,
			ERROR_SOCKET_FAIL_TCP,
			ERROR_SOCKET_FAIL_UDP,
			ERROR_BIND_FAIL_TCP,
			ERROR_BIND_FAIL_UDP,
			ERROR_ALREADY_INIT,
			ERROR_LISTEN_FAIL_TCP,
			ERROR_WSA
		};

		// Constructor and initialization functions
		NetworkControl( void );
		static SV_ERRORCODE InitializeServer( PCSTR listenPort, SV_PROTOCOL enabledProtocols = (SV_PROTOCOL) ( PACK_TCP | PACK_UDP ) );
		static SV_ERRORCODE InitializeClient( PCSTR destinationPort, PCSTR ipAddress, SV_PROTOCOL enabledProtocols = (SV_PROTOCOL) ( PACK_TCP | PACK_UDP ) );

		// Destructor and deinitialization function
		~NetworkControl( void );
		static void Deinitialize( void );

		static NetworkControl* GetSingleton( void );

		// TCP sending of packages, for sending to server and client respectively. Only use one of these. Overrides packet UDP flag.  
		int SendToServerTCP( Packet* packetToSend, bool deletePacket = true, int wsaFlags = 0 );
		int SendToClientTCP( CLIENT_ID privateID, Packet* packetToSend, bool deletePacket = true, int wsaFlags = 0 );

		// UDP sending of packages, for sending to server and client respectively. Only use one of these.   
		int SendToServerUDP( Packet* packetToSend, bool deletePacket = true, int wsaFlags = 0 );
		int SendToClientUDP( CLIENT_ID privateID, Packet* packetToSend, bool deletePacket = true, int wsaFlags = 0 );

		// Broadcasting of packages to all clients. For UDP sending, packet must specify this explicitly.
		void BroadcastToAll( Packet* packetToSend );

		// Client list operations
		Client* AddClient( CLIENT_ID publicId, CLIENT_ID privateId = 0, bool local = false, SOCKET socket = 0 );
		void RemoveClient( CLIENT_ID publicId, CLIENT_ID privateId = 0 );

		// Clientproxy list operations
		ClientProxy* AddClientProxy( CLIENT_ID publicId );
		void RemoveClientProxy( CLIENT_ID publicId );

		int ConnectToHost( void );

		HANDLE m_StopRequestedEvent; 
		HANDLE m_NetworkThreadHandle;
		Client* m_LocalClient;

		// Client containment
		std::vector< ClientContainer > m_ClientIds;
		std::vector< ClientContainer > m_ClientProxyIds;

		Client* m_PublicIdClientMap[USHRT_MAX];
		Client* m_PrivateIdClientMap[USHRT_MAX];
		ClientProxy* m_PublicIdClientProxyMap[USHRT_MAX];

		SOCKET m_SocketTCP, m_SocketUDP;
		static HANDLE* m_RecycleConnection;

	private:
		// Connection information
		SV_PROTOCOL m_EnabledProtocols;
		addrinfo* m_MyAddrInfoTCP, *m_MyAddrInfoUDP;
		WSADATA m_WSAData;

		CRITICAL_SECTION m_ClientAccessCSec;

	protected:
		static NetworkControl* m_Singleton;
		static int m_InitCount;

		friend DWORD WINAPI ListenerEntry( void* ptr );
		friend class Broadcaster;
		friend class UDPReceiver;

	private:
		// Protection against copying and assignment by overriding the copy constructor,
		// and overloading the assignment operator. Thanks for the tip Juul.

		NetworkControl( const NetworkControl& netInstance );
		const NetworkControl& operator=( const NetworkControl& netInstance );
	};

	// Extern entrypoints for every scenario
	extern DWORD WINAPI ServerEntry( void* ptr );
	extern DWORD WINAPI ClientEntry( void* ptr );
	extern DWORD WINAPI BroadcastEntry( void* ptr );
	extern DWORD WINAPI ListenerEntry( void* ptr );

	extern unsigned short RandomID( void );
};