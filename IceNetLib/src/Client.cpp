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
		m_StopEvent( TRUE ),
		m_Thread( 0 ),
		m_SocketTCP( socket ),
		m_PublicId( publicId ),
		m_PrivateId( privateId ),
		m_UDPInitialized( false ),
		m_SenderObject( 0 ),
		m_ReceiverObject( 0 ),
		m_HandlerObject( 0 ),
		m_AssociatedObject( 0 ),
		m_Local( local )
{
	m_SenderObject = new PacketSender( this );
	m_ReceiverObject = new PacketReceiver( this );

	if ( !( NetworkControl::GetSingleton()->GetFlags() & NetworkControl::HANDLER_SYNC ) )
	{
		m_HandlerObject = new PacketHandler( this );
	}

	// Create the KeepAlive thread
	m_Thread = new Thread( KeepAlive, this );
}

Client::~Client( void )
{
	delete m_SenderObject;
	delete m_ReceiverObject;
	delete m_HandlerObject;

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

	if ( m_SocketTCP != 0 )
	{
        closesocket( m_SocketTCP );
    }
}

THREAD_FUNC Client::KeepAlive( void* client )
{
	Client* cl = (Client*) client;

	// Simply wait till the client gives the stop signal and then..
	cl->m_StopEvent.Wait( INFINITE );

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
	m_AccessMutex.Lock();
	m_UDPOrigin = addr;
	m_AccessMutex.Unlock();
}

sockaddr Client::GetUDPOrigin( void )
{
	return m_UDPOrigin;
}

bool Client::CompareUDPOrigin( const sockaddr& addr )
{
	m_AccessMutex.Lock();

	if ( strcmp( addr.sa_data, m_UDPOrigin.sa_data ) == 0 && addr.sa_family == m_UDPOrigin.sa_family )
	{
		m_AccessMutex.Unlock();
		return true;
	}

	m_AccessMutex.Unlock();

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

Event& Client::GetStopEvent( void )
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

void Client::SetStop( void )
{
	m_StopEvent.Set();

	m_SenderObject->AddToQueue( 0 );
	m_HandlerObject->AddToQueue( 0 );
}
