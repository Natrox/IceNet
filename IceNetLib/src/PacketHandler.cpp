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
	if ( !( NetworkControl::GetSingleton()->GetFlags() & NetworkControl::HANDLER_SYNC ) )
	{
		m_Thread = new Thread( PacketHandlerLoop, this );
	}
}

PacketHandler::~PacketHandler( void )
{
	if ( NetworkControl::GetSingleton()->GetFlags() & NetworkControl::HANDLER_ASYNC )
	{
		AddToQueue( 0 );

		m_Thread->Wait( INFINITE );
		delete m_Thread;
	}

	// Delete all remaining packets. We do not need them.
	if ( m_PacketQueue.size() > 0 )
	{
		for ( unsigned int i = 0; i < m_PacketQueue.size(); i++ )
		{
			if ( m_PacketQueue.front() != 0 ) delete m_PacketQueue.front();
			m_PacketQueue.pop();
		}
	}
}

THREAD_FUNC PacketHandler::PacketHandlerLoop( void* packetHandler )
{
	if ( NetworkControl::GetSingleton()->GetFlags() & NetworkControl::HANDLER_SYNC )
	{
		return 1;
	}

	PacketHandler* ph = (PacketHandler*) packetHandler;

	while ( true )
	{
		ph->m_PacketQueueSemaphore.Wait( INFINITE );

		if ( NetworkControl::GetSingleton()->m_StopRequestedEvent.Wait( 0 ) == true ||
			ph->m_ParentClient->m_StopEvent.Wait( 0 ) == true )
		{
			break;
		}

		ph->m_PacketAdditionMutex.Lock();

		// Get a packet from the queue.
		Packet* handlePacket = ph->m_PacketQueue.front();
		ph->m_PacketQueue.pop();

		ph->m_PacketAdditionMutex.Unlock();

		// Handle the packet if its operation code is recognized
		OpCodeHandler::GetSingleton()->CallOpCodeFunction( handlePacket->GetOpCode(), handlePacket );

		delete handlePacket;
	}

	return 1;
}

void PacketHandler::AddToQueue( Packet* packet )
{
	m_PacketAdditionMutex.Lock();

	m_PacketQueue.push( packet );
	m_PacketQueueSemaphore.Notify();

	m_PacketAdditionMutex.Unlock();
}

unsigned int PacketHandler::HandlePackets( void )
{
	if ( !( NetworkControl::GetSingleton()->GetFlags() & NetworkControl::HANDLER_SYNC ) )
	{
		return 0;
	}

	unsigned int amount = 0;

	m_PacketAdditionMutex.Lock();

	while ( m_PacketQueue.size() > 0 )
	{
		// Get a packet from the queue.
		Packet* handlePacket = m_PacketQueue.front();
		m_PacketQueue.pop();

		// Handle the packet if its operation code is recognized
		OpCodeHandler::GetSingleton()->CallOpCodeFunction( handlePacket->GetOpCode(), handlePacket );

		delete handlePacket;
		amount++;
	}

	m_PacketAdditionMutex.Unlock();

	return amount;
}
