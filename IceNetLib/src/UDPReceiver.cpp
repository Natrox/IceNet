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

#include "UDPReceiver.h"
#include "NetworkControl.h"
#include "PacketHandler.h"
#include "Packet.h"
#include "Client.h"

#include "Modules.h"

using namespace IceNet;

UDPReceiver* UDPReceiver::m_Singleton = NULL;

namespace IceNet
{
	DWORD WINAPI UDPReceiverEntry( void* ptr )
	{
		// Get a new broadcaster object
		UDPReceiver* urObject = UDPReceiver::GetSingleton();
		urObject->m_UDPThreadHandle = GetCurrentThread();

		// Loop 'forever'
		urObject->ProcessLoop();

		// Delete the broadcaster
		delete urObject;
		UDPReceiver::m_Singleton = 0;

		return 1;
	}
};

UDPReceiver::UDPReceiver( void )
{}

UDPReceiver::~UDPReceiver( void )
{}

UDPReceiver* UDPReceiver::GetSingleton()
{
	if ( m_Singleton == 0 )
	{
		m_Singleton = new UDPReceiver();
	}

	return m_Singleton;
}

void UDPReceiver::ProcessLoop( void )
{
	NetworkControl* net = NetworkControl::GetSingleton();

	timeval timeout;
	fd_set readfs;

	while ( true )
	{
		const HANDLE waitHandles[] = { NetworkControl::GetSingleton()->m_StopRequestedEvent };

		DWORD waitObject = WaitForMultipleObjects( 1, waitHandles, false, 0 );

		if ( waitObject == WAIT_OBJECT_0 )
		{
			break;
		}

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		FD_ZERO( &readfs );
		FD_SET( net->m_SocketUDP, &readfs );

		int time = select( net->m_SocketUDP, &readfs, 0, 0, &timeout );
		if ( time == 0 || time == SOCKET_ERROR ) continue;

		Packet* pack = new Packet();

		char mnumber[ 4 ] = {0,0,0,0};

		unsigned int totalReceivableSize = sizeof( unsigned short ) * 2;
		unsigned int sizeLeft = totalReceivableSize;
		int sizeRead = 0;

		sockaddr orig;
		int b = sizeof ( orig );

		bool leave = false;

		while ( sizeLeft > 0 )
		{
			sizeRead = recvfrom( net->m_SocketUDP, mnumber + totalReceivableSize-sizeLeft, sizeLeft, MSG_PEEK, &orig, &b );
			sizeLeft -= sizeRead;

			if ( sizeRead <= 0 ) break;
		}

		unsigned short size = *( (unsigned short*) ( mnumber + 2 ) );
		
		if ( ( mnumber[0] != 'I' || mnumber[1] != 'S' ) )
		{
			delete pack;
			continue;
		}

		// Calculate the sizes
		totalReceivableSize = (unsigned short) size + sizeof( unsigned short );
		sizeLeft = totalReceivableSize;
		char* data = (char*) malloc( (size_t) totalReceivableSize );

		while ( sizeLeft > 0 )
		{
			sizeRead = recvfrom( net->m_SocketUDP, data + totalReceivableSize-sizeLeft, sizeLeft, 0, &orig, &b );
			if ( sizeRead == -1 ) { leave = true; break; }
			
			sizeLeft -= sizeRead;

			if ( sizeRead <= 0 )
			{
				break;
			}
		}

		if ( leave == true )
		{
			delete pack;
			free( data );

			continue;
		}

		if ( sizeRead <= 0 )
		{
			delete pack;
			free( data );

			continue;
		}

		pack->BorrowFromDataStream( data );

		bool packSafeToDelete = true;

		EnterCriticalSection( &net->m_ClientAccessCSec );

		Client* client = net->m_PrivateIdClientMap[ pack->GetClientPrivateId() ];
		
		if ( client != NULL )
		{
			if ( !( NetworkControl::GetSingleton()->GetFlags() & NetworkControl::HANDLER_SYNC ) ) 
			{
				client->GetHandlerObject()->AddToQueue( pack );
				packSafeToDelete = false;
			}
			
			if ( client->m_UDPInitialized == false )
			{
				client->m_UDPInitialized = true;
				client->m_UDPOrigin = orig;
			}
		}

		else if ( net->m_LocalClient != 0 )
		{
			client = net->m_LocalClient;
			
			if ( !( NetworkControl::GetSingleton()->GetFlags() & NetworkControl::HANDLER_SYNC ) ) 
			{
				client->GetHandlerObject()->AddToQueue( pack );
				packSafeToDelete = false;
			}
		}

		if ( ( NetworkControl::GetSingleton()->GetFlags() & NetworkControl::HANDLER_SYNC ) ) 
		{
			NetworkControl::GetSingleton()->GetPacketHandler()->AddToQueue( pack );
			packSafeToDelete = false;
		}

		if ( packSafeToDelete )
		{
			delete pack;
		}

		LeaveCriticalSection( &net->m_ClientAccessCSec );	
	}
}