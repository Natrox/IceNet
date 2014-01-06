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

#include "Client.h"
#include "Packet.h"
#include "OpCodeHandler.h"
#include "PacketSender.h"
#include "PacketReceiver.h"
#include "PacketHandler.h"
#include "NetworkControl.h"

#include "IceNetServer.h"
#include "IceNetTypedefs.h"

using namespace IceNet;

Client::Client( CLIENT_ID publicId, CLIENT_ID privateId, bool local, SOCKET socket ) :
		m_PublicId( publicId ),
		m_PrivateId( privateId ),
		m_SenderObject( NULL ),
		m_ReceiverObject( NULL ),
		m_HandlerObject( NULL ),
		m_AssociatedObject( NULL ),
		m_SocketTCP( socket ),
		m_Local( local ),
		m_UDPInitialized( false )
{
	InitializeCriticalSection( &m_AccessCSec );

	m_StopEvent = CreateEvent( NULL, true, false, NULL );

	m_SenderObject = new PacketSender( this );
	m_ReceiverObject = new PacketReceiver( this );

	if ( !( NetworkControl::GetSingleton()->GetFlags() & NetworkControl::HANDLER_SYNC ) )
	{
		m_HandlerObject = new PacketHandler( this );
	}

	// Create the KeepAlive thread
	m_ThreadHandle = CreateThread( NULL, NULL, KeepAlive, this, NULL, NULL );
}

Client::~Client( void )
{
	delete m_SenderObject;
	delete m_ReceiverObject;
	delete m_HandlerObject;

	DeleteCriticalSection( &m_AccessCSec );
	CloseHandle( m_StopEvent );

	if ( m_Local == false )
	{
		// Check for a callback function and call it if it is not 0
		VOID_WITH_CLIENT_PARAM fun = ServerSide::GetOnRemoveClient();
		if ( fun != 0 ) fun( this );

		// Broadcast the removal of this client to all other clients.
		Packet* broadcast = new Packet();

		broadcast->SetOpCodeInternal( OpCodeHandler::REMOVE_CLIENT );
		broadcast->AddDataStreaming<unsigned short>( m_PublicId );

		NetworkControl::GetSingleton()->BroadcastToAll( broadcast );
	}

	if ( m_SocketTCP != NULL ) closesocket( m_SocketTCP );
}

DWORD WINAPI Client::KeepAlive( void* client )
{
	Client* cl = (Client*) client;

	// Simply wait till the client gives the stop signal and then..
	WaitForSingleObject( cl->m_StopEvent, INFINITE );

	// ..remove the client from the network controls.
	NetworkControl::GetSingleton()->RemoveClient( cl->m_PublicId, cl->m_PrivateId );

	return 1;
}

void Client::SetSocket( const SOCKET& socket )
{
	m_SocketTCP = socket;
}

SOCKET Client::GetSocket( void )
{
	return m_SocketTCP;
}

void Client::SetAssociatedObject( PTR_ANYTHING object )
{
	m_AssociatedObject = object;
}

void Client::SetUDPOrigin( const sockaddr& addr )
{
	EnterCriticalSection( &m_AccessCSec );
	m_UDPOrigin = addr;
	LeaveCriticalSection( &m_AccessCSec );
}

sockaddr Client::GetUDPOrigin( void )
{
	return m_UDPOrigin;
}

bool Client::CompareUDPOrigin( const sockaddr& addr )
{
	EnterCriticalSection( &m_AccessCSec );

	if ( strcmp( addr.sa_data, m_UDPOrigin.sa_data ) == 0 && addr.sa_family == m_UDPOrigin.sa_family )
	{
		LeaveCriticalSection( &m_AccessCSec );
		return true;
	}

	LeaveCriticalSection( &m_AccessCSec );

	return false;
}

PacketSender* Client::GetSenderObject( void )
{
	return m_SenderObject;
}

PacketReceiver* Client::GetReceiverObject( void )
{
	return m_ReceiverObject;
}

PacketHandler* Client::GetHandlerObject( void )
{
	return m_HandlerObject;
}

bool Client::IsLocal( void )
{
	return m_Local;
}

HANDLE Client::GetStopEvent( void )
{
	return m_StopEvent;
}

std::string Client::GetIPAddress( void )
{
	return m_IP;
}

CLIENT_ID Client::GetPublicId( void )
{
	return m_PublicId;
}

CLIENT_ID Client::GetPrivateId( void )
{
	return m_PrivateId;
}
