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

#include "PacketReceiver.h"
#include "PacketHandler.h"
#include "NetworkControl.h"
#include "Packet.h"
#include "Client.h"

using namespace IceNet;

PacketReceiver::PacketReceiver( Client* parentClient ) :
			  m_ParentClient( parentClient )
{
	m_ThreadHandle = CreateThread( NULL, NULL, PacketReceiverLoop, this, NULL, NULL );
}

PacketReceiver::~PacketReceiver( void )
{
	WaitForSingleObject( m_ThreadHandle, INFINITE );

	CloseHandle( m_ThreadHandle );
}

DWORD WINAPI PacketReceiver::PacketReceiverLoop( void* packetReceiver )
{
	PacketReceiver* pr = (PacketReceiver*) packetReceiver;

	while ( true )
	{
		const HANDLE waitHandles[] = { NetworkControl::GetSingleton()->m_StopRequestedEvent, pr->m_ParentClient->m_StopEvent };

		DWORD waitObject = WaitForMultipleObjects( 2, waitHandles, false, 0 );

		if ( waitObject == WAIT_OBJECT_0 || waitObject == WAIT_OBJECT_0 + 1 )
		{
			break;
		}

		Packet* pack = new Packet();

		char mnumber[ 4 ] = {0,0,0,0};

		unsigned int totalReceivableSize = sizeof( unsigned short ) * 2;
		unsigned int sizeLeft = totalReceivableSize;
		int sizeRead = 0;

		while ( sizeLeft > 0 )
		{
			sizeRead = recv( pr->m_ParentClient->m_SocketTCP, mnumber + totalReceivableSize-sizeLeft, sizeLeft, MSG_PEEK );
			sizeLeft -= sizeRead;

			if ( sizeRead <= 0 ) break;
		}

		unsigned short size = *( (unsigned short*) ( mnumber + 2 ) );

		if ( mnumber[0] != 'I' || mnumber[1] != 'S' )
		{
			delete pack;

			SetEvent( pr->m_ParentClient->m_StopEvent );
			return 1;
		}

		// Calculate the sizes
		totalReceivableSize = (unsigned short) size + sizeof( unsigned short );
		sizeLeft = totalReceivableSize;
		char* data = (char*) malloc( (size_t) totalReceivableSize );
		
		// Read the rest of the data.
		while ( sizeLeft > 0 )
		{
			sizeRead = recv( pr->m_ParentClient->m_SocketTCP, data + totalReceivableSize-sizeLeft, sizeLeft, 0 );
			sizeLeft -= sizeRead;

			if ( sizeRead <= 0 )
			{
				break;
			}
		}

		if ( sizeRead <= 0 )
		{
			delete pack;

			free( data );
			SetEvent( pr->m_ParentClient->m_StopEvent );
			return 1;
		}

		pack->SetFromDataStream( data, totalReceivableSize );

		if ( !pack->GetUDPEnabled() )
		{
			pr->m_ParentClient->GetHandlerObject()->AddToQueue( pack );
		}

		else
		{
			delete pack;
		}

		free( data );
	}

	return 1;
}