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

#include "NetworkControl.h"
#include "Packet.h"
#include "Client.h"
#include "ClientProxy.h"
#include "Broadcaster.h"

#include "PacketHandler.h"

#include <assert.h>

using namespace IceNet;

HANDLE* NetworkControl::m_RecycleConnection = NULL;
NetworkControl* NetworkControl::m_Singleton = NULL;
int NetworkControl::m_InitCount = NULL;

NetworkControl::NetworkControl( void ) :
				m_Flags( (Flags) ( PROTOCOL_TCP | PROTOCOL_UDP ) ),
				m_SocketTCP( NULL ), m_SocketUDP( NULL ),
				m_NetworkThreadHandle( NULL ),
				m_StopRequestedEvent( NULL ),
				m_LocalClient( NULL )
{
	InitializeCriticalSection( &m_ClientAccessCSec );

	// Initialize arrays to 0
	memset( m_PublicIdClientMap, 0, sizeof( void* ) * USHRT_MAX );
	memset( m_PrivateIdClientMap, 0, sizeof( void* ) * USHRT_MAX );
	memset( m_PublicIdClientProxyMap, 0, sizeof( void* ) * USHRT_MAX );

	m_StopRequestedEvent = CreateEvent( NULL, true, false, NULL );
	if ( m_RecycleConnection == 0 ) 
	{
		m_RecycleConnection = new HANDLE();
		(*m_RecycleConnection) = CreateEvent( NULL, false, false, NULL );
	}
}

NetworkControl::ErrorCodes NetworkControl::InitializeServer( PCSTR listenPort, Flags flags )
{
	// Return if the networking has been initialized already
	if ( m_InitCount != 0 ) return ERROR_ALREADY_INIT;

	m_Singleton = new NetworkControl();
	m_Singleton->m_Flags = flags;

	if ( m_Singleton->m_Flags & HANDLER_SYNC )
	{
		m_Singleton->m_PacketHandler = new PacketHandler( 0 );
	}

	else
	{
		m_Singleton->m_PacketHandler = 0;
	}
	
	// Try to start WinSock
	if ( WSAStartup( MAKEWORD( 2, 0 ), &m_Singleton->m_WSAData ) == -1 )
	{
		// WinSock fails

		delete m_Singleton;

		return ERROR_WSA;
	}

	struct addrinfo hints;

	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Use local address to construct an addrinfo
	getaddrinfo( "0.0.0.0", listenPort, &hints, &m_Singleton->m_MyAddrInfoTCP );
	
	// Create the TCP socket
	m_Singleton->m_SocketTCP = socket( m_Singleton->m_MyAddrInfoTCP->ai_family, m_Singleton->m_MyAddrInfoTCP->ai_socktype, m_Singleton->m_MyAddrInfoTCP->ai_protocol );

	if ( m_Singleton->m_SocketTCP == -1 )
	{
		// TCP socket fails

		delete m_Singleton;

		return ERROR_SOCKET_FAIL_TCP;
	}

	// Bind the TCP socket
	if ( bind( m_Singleton->m_SocketTCP, m_Singleton->m_MyAddrInfoTCP->ai_addr, (int) m_Singleton->m_MyAddrInfoTCP->ai_addrlen ) == -1 )
	{
		delete m_Singleton;

		return ERROR_BIND_FAIL_TCP;
	}

	// Start listening for connections
	if ( listen( m_Singleton->m_SocketTCP, BACKLOG ) == -1 )
	{
		delete m_Singleton;

		return ERROR_LISTEN_FAIL_TCP;
	}

	// If UDP has been enabled, go ahead and do all this for UDP
	if ( flags & PROTOCOL_UDP )
	{
		memset( &hints, 0, sizeof( hints ) );
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM; // For UDP
		hints.ai_protocol = IPPROTO_UDP;
		hints.ai_flags = AI_PASSIVE;

		getaddrinfo( "0.0.0.0", listenPort, &hints, &m_Singleton->m_MyAddrInfoUDP );
		m_Singleton->m_SocketUDP = socket( m_Singleton->m_MyAddrInfoUDP->ai_family, m_Singleton->m_MyAddrInfoUDP->ai_socktype, m_Singleton->m_MyAddrInfoUDP->ai_protocol );

		if ( m_Singleton->m_SocketUDP == -1 )
		{
			delete m_Singleton;

			return ERROR_SOCKET_FAIL_UDP;
		}

		if ( bind( m_Singleton->m_SocketUDP, m_Singleton->m_MyAddrInfoUDP->ai_addr, (int) m_Singleton->m_MyAddrInfoUDP->ai_addrlen ) == -1 )
		{
			delete m_Singleton;

			return ERROR_BIND_FAIL_UDP;
		}
	}

	m_Singleton->m_NetworkThreadHandle = CreateThread( NULL, NULL, ServerEntry, NULL, NULL, NULL );

	++m_InitCount;

	// No error
	return ERROR_NONE;
}

NetworkControl::ErrorCodes NetworkControl::InitializeClient( PCSTR destinationPort, PCSTR ipAddress, Flags flags )
{
	if ( m_InitCount != 0 ) return ERROR_ALREADY_INIT;

	m_Singleton = new NetworkControl();
	m_Singleton->m_Flags = flags;

	if ( m_Singleton->m_Flags & HANDLER_SYNC )
	{
		m_Singleton->m_PacketHandler = new PacketHandler( 0 );
	}

	else
	{
		m_Singleton->m_PacketHandler = 0;
	}
	
	if ( WSAStartup( MAKEWORD( 2, 0 ), &m_Singleton->m_WSAData ) == -1 )
	{
		delete m_Singleton;

		return ERROR_WSA;
	}

	struct addrinfo hints;

	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	struct sockaddr_in sa;

	// store this IP address in sa:
	int valid = inet_pton( AF_INET, ipAddress, &(sa.sin_addr) );

	if ( valid != 0 )
	{
		getaddrinfo( ipAddress, destinationPort, &hints, &m_Singleton->m_MyAddrInfoTCP );
		m_Singleton->m_SocketTCP = socket( m_Singleton->m_MyAddrInfoTCP->ai_family, m_Singleton->m_MyAddrInfoTCP->ai_socktype, m_Singleton->m_MyAddrInfoTCP->ai_protocol );
	}

	if ( m_Singleton->m_SocketTCP == -1 || valid == 0 )
	{
		delete m_Singleton;

		return ERROR_SOCKET_FAIL_TCP;
	}

	if ( flags & PROTOCOL_UDP )
	{
		memset( &hints, 0, sizeof( hints ) );
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
		hints.ai_flags = AI_PASSIVE;

		getaddrinfo( ipAddress, destinationPort, &hints, &m_Singleton->m_MyAddrInfoUDP );
		m_Singleton->m_SocketUDP = socket( m_Singleton->m_MyAddrInfoUDP->ai_family, m_Singleton->m_MyAddrInfoUDP->ai_socktype, m_Singleton->m_MyAddrInfoUDP->ai_protocol );

		if ( m_Singleton->m_SocketUDP == -1 )
		{
			delete m_Singleton;

			return ERROR_SOCKET_FAIL_UDP;
		}
	}

	m_Singleton->m_NetworkThreadHandle = CreateThread( NULL, NULL, ClientEntry, NULL, NULL, NULL );

	++m_InitCount;

	return ERROR_NONE;
}

NetworkControl::~NetworkControl( void )
{
	closesocket( m_SocketTCP );
	closesocket( m_SocketUDP );

	SetEvent( m_StopRequestedEvent );

	// Wait for everything else to clean up and shut down
	WaitForSingleObject( m_NetworkThreadHandle, INFINITE );

	DeleteCriticalSection( &m_ClientAccessCSec );

	CloseHandle( m_NetworkThreadHandle );
	CloseHandle( m_StopRequestedEvent );

	WSACleanup();

	// Delete all clients
	if ( m_ClientIds.size() > 0 ) 
	{
		for ( unsigned int i = 0; i < m_ClientIds.size(); ++i )
		{
			delete m_PublicIdClientMap[ m_ClientIds[i].publicId ];
		}
	}

	// Delete all clientproxies
	if ( m_ClientProxyIds.size() > 0 ) 
	{
		for ( unsigned int i = 0; i < m_ClientProxyIds.size(); ++i )
		{
			delete m_PublicIdClientProxyMap[ m_ClientProxyIds[i].publicId ];
		}
	}

	if ( m_Singleton->m_Flags & HANDLER_SYNC )
	{
		delete m_Singleton->m_PacketHandler;
	}
}

void NetworkControl::Deinitialize( void )
{
	assert( m_InitCount > 0 );

	delete m_Singleton;
	m_Singleton = 0;

	--m_InitCount;

	SetEvent( m_RecycleConnection );
}

NetworkControl* NetworkControl::GetSingleton( void )
{
	return m_Singleton;
}

int NetworkControl::SendToServerTCP( Packet* packetToSend, bool deletePacket, int wsaFlags )
{	
	// Calculate the real packet size (size of the string)
	unsigned short size = packetToSend->GetSize() + sizeof( unsigned short );

	// Get the packet in string format
	char* buf = packetToSend->GetDataStream();

	unsigned int sizeLeft = size;
	int sizeRead = 0;

	// Send until no bytes are left
	while ( sizeLeft > 0 )
	{
		sizeRead = send( m_SocketTCP, &buf[size-sizeLeft], sizeLeft, wsaFlags );
		sizeLeft -= sizeRead;

		// No bytes left or error
		if ( sizeRead <= 0 )
		{
			break;
		}
	}
	
	if ( deletePacket ) delete packetToSend;

	return sizeRead;
}

int NetworkControl::SendToClientTCP( CLIENT_ID privateID, Packet* packetToSend, bool deletePacket, int wsaFlags )
{
	unsigned short size = packetToSend->GetSize() + sizeof( unsigned short );
	char* buf = packetToSend->GetDataStream();

	unsigned int sizeLeft = size;
	int sizeRead = 0;

	while ( sizeLeft > 0 && m_PrivateIdClientMap[privateID] != 0 )
	{
		sizeRead = send( m_PrivateIdClientMap[privateID]->GetSocket(), &buf[size-sizeLeft], sizeLeft, wsaFlags );
		sizeLeft -= sizeRead;

		if ( sizeRead <= 0 )
		{
			break;
		}
	}
	
	if ( deletePacket ) delete packetToSend;

	return sizeRead;
}

int NetworkControl::SendToServerUDP( Packet* packetToSend, bool deletePacket, int wsaFlags )
{	
	// Calculate the real packet size (size of the string)
	unsigned short size = 256;
	unsigned short realSize = packetToSend->GetSize() + sizeof( unsigned short );

	// Get the packet in string format
	char* buf = packetToSend->GetDataStream();
	char udpBuffer[256];

	memcpy( udpBuffer, buf, realSize > 256 ? 256 : realSize );

	unsigned int sizeLeft = size;
	int sizeRead = 0;

	// Send until no bytes are left
	sendto( m_SocketUDP, udpBuffer, (int) sizeLeft, wsaFlags, m_MyAddrInfoUDP->ai_addr, (int) m_MyAddrInfoUDP->ai_addrlen );

	if ( deletePacket ) delete packetToSend;

	return 1;
}

int NetworkControl::SendToClientUDP( CLIENT_ID privateID, Packet* packetToSend, bool deletePacket, int wsaFlags )
{
	if ( m_PrivateIdClientMap[privateID] == 0 )
	{
		if ( deletePacket ) delete packetToSend;
		return -1;
	}

	sockaddr udpOrig = m_PrivateIdClientMap[privateID]->GetUDPOrigin();

	unsigned short size = 256;
	unsigned short realSize = packetToSend->GetSize() + sizeof( unsigned short );

	char* buf = packetToSend->GetDataStream();
	char udpBuffer[256];

	memcpy( udpBuffer, buf, realSize > 256 ? 256 : realSize );

	unsigned int sizeLeft = size;
	int sizeRead = 0;

	if ( m_PrivateIdClientMap[privateID] != 0 && m_PrivateIdClientMap[privateID]->m_UDPInitialized == true )
	{
		const int b = sizeof ( sockaddr );
		sizeRead = sendto( m_SocketUDP, buf, 256, wsaFlags, &udpOrig, b );
	}
	
	if ( deletePacket ) delete packetToSend;

	return 1;
}

void NetworkControl::BroadcastToAll( Packet* packetToSend )
{
	Broadcaster::GetSingleton()->AddToList( packetToSend );
}

Client* NetworkControl::AddClient( CLIENT_ID publicId, CLIENT_ID privateId, bool local, SOCKET socket )
{
	assert( publicId != 0 );

	EnterCriticalSection( &m_ClientAccessCSec );

	// Create a new client container object
	Client* newClientObj;

	newClientObj = new Client( publicId, privateId, local, socket );
	ClientContainer newClient = { publicId, privateId, newClientObj };

	// Add the client to the maps. Private ID is not required.
	m_PublicIdClientMap[ publicId ] = newClientObj;
	if ( privateId != 0 ) m_PrivateIdClientMap[ privateId ] = newClientObj;

	m_ClientIds.push_back( newClient );

	LeaveCriticalSection( &m_ClientAccessCSec );

	return newClientObj;
}

void NetworkControl::RemoveClient( CLIENT_ID publicId, CLIENT_ID privateId )
{
	assert( publicId != 0 );

	EnterCriticalSection( &m_ClientAccessCSec );

	if ( m_ClientIds.size() > 0 )
	{
		for ( unsigned int i = 0; i < m_ClientIds.size(); i++ )
		{
			if ( m_ClientIds[i].publicId == publicId )
			{
				// Delete either by private ID or public ID
				if ( m_ClientIds[i].privateId != 0 )
				{
					delete m_PrivateIdClientMap[ m_ClientIds[i].privateId ];
					m_PrivateIdClientMap[ m_ClientIds[i].privateId ] = NULL;
					m_PublicIdClientMap[ m_ClientIds[i].publicId ] = NULL;
				}

				else
				{
					delete m_PublicIdClientMap[ m_ClientIds[i].publicId ];
					if ( m_ClientIds[i].privateId != 0 ) m_PrivateIdClientMap[ m_ClientIds[i].privateId ] = NULL;
					m_PublicIdClientMap[ m_ClientIds[i].publicId ] = NULL;
				}

				// Remove the client from the ID list
				m_ClientIds[i] = m_ClientIds.back();
				m_ClientIds.pop_back();

				break;
			}
		}
	}

	LeaveCriticalSection( &m_ClientAccessCSec );
}

ClientProxy* NetworkControl::AddClientProxy( CLIENT_ID publicId )
{
	assert( publicId != 0 );

	EnterCriticalSection( &m_ClientAccessCSec );

	// Create a new client container object
	ClientProxy* newClientObj;

	newClientObj = new ClientProxy( publicId );
	ClientContainer newClient = { publicId, 0, newClientObj };

	// Add the client to the maps. Private ID is not required.
	m_PublicIdClientProxyMap[ publicId ] = newClientObj;

	m_ClientProxyIds.push_back( newClient );

	LeaveCriticalSection( &m_ClientAccessCSec );

	return newClientObj;
}

void NetworkControl::RemoveClientProxy( CLIENT_ID publicId )
{
	assert( publicId != 0 );

	EnterCriticalSection( &m_ClientAccessCSec );

	if ( m_ClientProxyIds.size() > 0 )
	{
		for ( unsigned int i = 0; i < m_ClientProxyIds.size(); i++ )
		{
			if ( m_ClientProxyIds[i].publicId == publicId )
			{
				delete m_PublicIdClientProxyMap[ m_ClientProxyIds[i].publicId ];
				m_PublicIdClientProxyMap[ m_ClientProxyIds[i].publicId ] = NULL;

				// Remove the client from the ID list
				m_ClientProxyIds[i] = m_ClientProxyIds.back();
				m_ClientProxyIds.pop_back();

				break;
			}
		}
	}

	LeaveCriticalSection( &m_ClientAccessCSec );
}

int NetworkControl::ConnectToHost( void )
{
	return connect( m_SocketTCP, m_MyAddrInfoTCP->ai_addr, (int) m_MyAddrInfoTCP->ai_addrlen );
}

unsigned int NetworkControl::GetFlags( void )
{
	return m_Flags;
}

PacketHandler* NetworkControl::GetPacketHandler( void )
{
	return m_PacketHandler;
}
