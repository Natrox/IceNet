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

#include "PacketHandler.h"
#include "NetworkControl.h"
#include "OpCodeHandler.h"
#include "Client.h"
#include "Packet.h"

using namespace IceNet;

PacketHandler::PacketHandler( Client* parentClient ) :
			  m_ParentClient( parentClient )
{
	InitializeCriticalSection( &m_PacketAdditionCSec );

	if ( !( NetworkControl::GetSingleton()->GetFlags() & NetworkControl::HANDLER_SYNC ) )
	{
		m_PacketQueueSemaphore = CreateSemaphore( NULL, NULL, LONG_MAX, NULL );
		m_ThreadHandle = CreateThread( NULL, NULL, PacketHandlerLoop, this, NULL, NULL );
	}
}

PacketHandler::~PacketHandler( void )
{
	if ( NetworkControl::GetSingleton()->GetFlags() & NetworkControl::HANDLER_ASYNC )
	{
		WaitForSingleObject( m_ThreadHandle, INFINITE );

		CloseHandle( m_ThreadHandle );
		CloseHandle( m_PacketQueueSemaphore );
	}

	DeleteCriticalSection( &m_PacketAdditionCSec );

	// Delete all remaining packets. We do not need them.
	if ( m_PacketQueue.size() > 0 )
	{
		for ( unsigned int i = 0; i < m_PacketQueue.size(); i++ )
		{
			delete m_PacketQueue.front();
			m_PacketQueue.pop();
		}
	}
}

DWORD WINAPI PacketHandler::PacketHandlerLoop( void* packetHandler )
{
	if ( NetworkControl::GetSingleton()->GetFlags() & NetworkControl::HANDLER_SYNC )
	{
		return 1;
	}

	PacketHandler* ph = (PacketHandler*) packetHandler;

	while ( true )
	{
		const HANDLE waitHandles[] = { ph->m_PacketQueueSemaphore, NetworkControl::GetSingleton()->m_StopRequestedEvent, ph->m_ParentClient->m_StopEvent };

		DWORD waitObject = WaitForMultipleObjects( 3, waitHandles, false, INFINITE );

		if ( waitObject == WAIT_OBJECT_0 + 1 || waitObject == WAIT_OBJECT_0 + 2 )
		{
			break;
		}

		EnterCriticalSection( &ph->m_PacketAdditionCSec );

		// Get a packet from the queue.
		Packet* handlePacket = ph->m_PacketQueue.front();
		ph->m_PacketQueue.pop();

		LeaveCriticalSection( &ph->m_PacketAdditionCSec );

		// Handle the packet if its operation code is recognized
		PACKET_HANDLING_FUNCTION fun = OpCodeHandler::GetSingleton()->GetOpCodeFunction( handlePacket->GetOpCode() );
		if ( fun != NULL ) fun( handlePacket );
		
		delete handlePacket;
	}

	return 1;
}

void PacketHandler::AddToQueue( Packet* packet )
{
	EnterCriticalSection( &m_PacketAdditionCSec );

	m_PacketQueue.push( packet );
	ReleaseSemaphore( m_PacketQueueSemaphore, 1, 0 );

	LeaveCriticalSection( &m_PacketAdditionCSec );
}

unsigned int PacketHandler::HandlePackets( void )
{
	if ( !( NetworkControl::GetSingleton()->GetFlags() & NetworkControl::HANDLER_SYNC ) )
	{
		return 0;
	}

	unsigned int amount = 0;

	EnterCriticalSection( &m_PacketAdditionCSec );

	while ( m_PacketQueue.size() > 0 )
	{
		// Get a packet from the queue.
		Packet* handlePacket = m_PacketQueue.front();
		m_PacketQueue.pop();

		// Handle the packet if its operation code is recognized
		PACKET_HANDLING_FUNCTION fun = OpCodeHandler::GetSingleton()->GetOpCodeFunction( handlePacket->GetOpCode() );
		if ( fun != NULL ) fun( handlePacket );
		
		delete handlePacket;
		amount++;
	}

	LeaveCriticalSection( &m_PacketAdditionCSec );

	return amount;
}