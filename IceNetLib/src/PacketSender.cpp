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

#include "PacketSender.h"
#include "NetworkControl.h"
#include "Packet.h"
#include "Client.h"

using namespace IceNet;

PacketSender::PacketSender( Client* parentClient ) :
			  m_ParentClient( parentClient )
{
	InitializeCriticalSection( &m_PacketAdditionCSec );

	m_PacketQueueSemaphore = CreateSemaphore( NULL, NULL, LONG_MAX, NULL );
	m_ThreadHandle = CreateThread( NULL, NULL, PacketSenderLoop, this, NULL, NULL );
}

PacketSender::~PacketSender( void )
{
	WaitForSingleObject( m_ThreadHandle, INFINITE );

	CloseHandle( m_ThreadHandle );
	CloseHandle( m_PacketQueueSemaphore );

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

DWORD WINAPI PacketSender::PacketSenderLoop( void* packetSender )
{
	PacketSender* ps = (PacketSender*) packetSender;

	while ( true )
	{
		const HANDLE waitHandles[] = { ps->m_PacketQueueSemaphore, NetworkControl::GetSingleton()->m_StopRequestedEvent, ps->m_ParentClient->m_StopEvent };

		DWORD waitObject = WaitForMultipleObjects( 3, waitHandles, false, INFINITE );

		if ( waitObject == WAIT_OBJECT_0 + 1 || waitObject == WAIT_OBJECT_0 + 2 )
		{
			break;
		}

		EnterCriticalSection( &ps->m_PacketAdditionCSec );
		
		// Get a packet from the queue.
		Packet* outgoingPacket = ps->m_PacketQueue.front();
		ps->m_PacketQueue.pop();

		LeaveCriticalSection( &ps->m_PacketAdditionCSec );

		if ( ps->m_ParentClient->IsLocal() == false )
		{
			// If no UDP
			if ( outgoingPacket->GetUDPEnabled() == false )
			{
				int code = NetworkControl::GetSingleton()->SendToClientTCP( ps->m_ParentClient->m_PrivateId, outgoingPacket );
			
				if ( code <= 0 )
				{
					return 1;
				}
			}

			// thus TCP
			else
			{
				int code = NetworkControl::GetSingleton()->SendToClientUDP( ps->m_ParentClient->m_PrivateId, outgoingPacket );
			
				if ( code <= 0 )
				{
					return 1;
				}
			}
		}

		else
		{
			if ( outgoingPacket->GetUDPEnabled() == false )
			{
				int code = NetworkControl::GetSingleton()->SendToServerTCP( outgoingPacket );
			
				if ( code <= 0 )
				{
					return 1;
				}
			}

			else
			{
				int code = NetworkControl::GetSingleton()->SendToServerUDP( outgoingPacket );
			
				if ( code <= 0 )
				{
					return 1;
				}
			}
		}
	}

	return 1;
}

void PacketSender::AddToQueue( Packet* packet )
{
	EnterCriticalSection( &m_PacketAdditionCSec );

	m_PacketQueue.push( packet );
	ReleaseSemaphore( m_PacketQueueSemaphore, 1, 0 );

	LeaveCriticalSection( &m_PacketAdditionCSec );
}