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

#include "Platforms.h"
#include "Threading.h"

#include <vector>
#include "ClientContainer.h"

/*
	ICECAT Networking		v2.0

	class NetworkControl:

	This class is the main interface for all networking activities (Internally).
	Caution advised.
*/

#define CONNECTION_BACKLOG 8

namespace IceNet
{
	typedef unsigned short CLIENT_ID;

	// Prototypes
	class Client;
	class ClientProxy;
	class Packet;
	class PacketHandler;

	void ReceiveID( Packet* pack );
	void AddRemoteClient( Packet* pack );
	void RemoveRemoteClient( Packet* pack );

	class NetworkControl
	{
	public:
		// TCP is always on, UDP is enabled by default
		enum Flags
		{
			// Protocols
			PROTOCOL_TCP = 0,
			PROTOCOL_UDP = 1,

			// Handler settings,
			HANDLER_ASYNC = 0,			// Packet handling is done asynchronously. Users are expected to make opcode functions threadsafe.
			HANDLER_SYNC = 2			// Packet handling is invoked by the user. Opcode functions run in the same thread HandlePackets() was called from.
		};

		// Error codes in case things fail
		enum ErrorCodes
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

		static NetworkControl* GetSingleton( void );
		unsigned int GetFlags( void );

	public:
		// Initialization functions
		static ErrorCodes InitializeServer( const char* listenPort, Flags enabledProtocols = (Flags) ( PROTOCOL_TCP | PROTOCOL_UDP ) );
		static ErrorCodes InitializeClient( const char* destinationPort, const char* ipAddress, Flags enabledProtocols = (Flags) ( PROTOCOL_TCP | PROTOCOL_UDP ) );

		// Destructor and deinitialization function
		~NetworkControl( void );
		static void Deinitialize( void );

		PacketHandler* GetPacketHandler( void );

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

		// Set ID's
		void SetPublicId( CLIENT_ID id );
		void SetPrivateId( CLIENT_ID id );

		// Flags
		int ConnectToHost( void );

		Event m_StopRequestedEvent;
		Thread* m_NetworkThread;
		Client* m_LocalClient;

		// Client containment
		std::vector< ClientContainer > m_ClientIds;
		std::vector< ClientContainer > m_ClientProxyIds;

		Client* m_PublicIdClientMap[USHRT_MAX];
		Client* m_PrivateIdClientMap[USHRT_MAX];
		ClientProxy* m_PublicIdClientProxyMap[USHRT_MAX];

		SOCKET m_SocketTCP, m_SocketUDP;
		static Event* m_RecycleConnection;

	private:
		// Connection information
		addrinfo* m_MyAddrInfoTCP, *m_MyAddrInfoUDP;
#ifdef _WIN32
		WSADATA m_WSAData;
#endif
		Flags m_Flags;

		// Packet handler for HANDLER_SYNC
		PacketHandler* m_PacketHandler;

		Mutex m_ClientAccessMutex;

	protected:
		static NetworkControl* m_Singleton;
		static int m_InitCount;

		friend THREAD_FUNC ListenerEntry( void* ptr );
		friend class Broadcaster;
		friend class UDPReceiver;
		friend class PacketSender;
		friend class PacketReceiver;
		friend class PacketHandler;

	private:
		friend void ReceiveID( Packet* pack );
		friend void AddRemoteClient( Packet* pack );
		friend void RemoveRemoteClient( Packet* pack );

	private:
		// Protection against copying and assignment by overriding the copy constructor,
		// and overloading the assignment operator. Thanks for the tip Juul.
		NetworkControl( void );
		NetworkControl( const NetworkControl& netInstance );
		const NetworkControl& operator=( const NetworkControl& netInstance );
	};

	// Extern entrypoints for every scenario
	extern THREAD_FUNC ServerEntry( void* ptr );
	extern THREAD_FUNC ClientEntry( void* ptr );
	extern THREAD_FUNC BroadcastEntry( void* ptr );
	extern THREAD_FUNC ListenerEntry( void* ptr );

	extern unsigned short RandomID( void );
};
