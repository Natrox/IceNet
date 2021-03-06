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
	m_Thread = new Thread( PacketReceiverLoop, this );
}

PacketReceiver::~PacketReceiver( void )
{
	m_Thread->Wait( INFINITE );
	delete m_Thread;
}

THREAD_FUNC PacketReceiver::PacketReceiverLoop( void* packetReceiver )
{
	PacketReceiver* pr = (PacketReceiver*) packetReceiver;

	while ( true )
	{
		if ( NetworkControl::GetSingleton()->m_StopRequestedEvent.Wait( 0 ) == true ||
			pr->m_ParentClient->m_StopEvent.Wait( 0 ) == true )
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

			pr->m_ParentClient->m_StopEvent.Set();
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
			pr->m_ParentClient->m_StopEvent.Set();
			return 1;
		}

		pack->BorrowFromDataStream( data );

		if ( !( NetworkControl::GetSingleton()->GetFlags() & NetworkControl::HANDLER_SYNC ) )
		{
			pr->m_ParentClient->GetHandlerObject()->AddToQueue( pack );
		}

		else
		{
			NetworkControl::GetSingleton()->GetPacketHandler()->AddToQueue( pack );
		}
	}

	return 1;
}
