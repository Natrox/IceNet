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
#include "Broadcaster.h"
#include "Client.h"
#include "PacketSender.h"

#include "Modules.h"

using namespace IceNet;

Broadcaster* Broadcaster::m_Singleton = 0;

namespace IceNet
{
	THREAD_FUNC BroadcastEntry( void* ptr )
	{
		// Get a new broadcaster object
		Broadcaster* bcObject = Broadcaster::GetSingleton();

		// Loop 'forever'
		bcObject->ProcessLoop();

		return 1;
	}
};

Broadcaster::Broadcaster( void )
    :
   m_StopRequestedEvent( TRUE )
{}

Broadcaster::~Broadcaster( void )
{
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

Broadcaster* Broadcaster::GetSingleton()
{
    if ( m_Singleton == 0 )
    {
        m_Singleton = new Broadcaster();
    }

    return m_Singleton;
}

void Broadcaster::AddToList( Packet* sourcePacket )
{
    // Add a new packet to the queue
    m_PacketAdditionMutex.Lock();

    m_PacketQueue.push( sourcePacket );
    m_PacketQueueSemaphore.Notify();

    m_PacketAdditionMutex.Unlock();
}

void Broadcaster::ProcessLoop( void )
{
    while ( true )
    {
        // Wait for either
        m_PacketQueueSemaphore.Wait( INFINITE );

        // If a stop has been requested...
        if ( m_StopRequestedEvent.Wait( 0 ) )
        {
            // ..close shop
            return;
        }

        m_PacketAdditionMutex.Lock();

        // Get the front packet and remove it from the queue
        Packet* outgoingPacket = 0;

        if ( m_PacketQueue.size() > 0 )
        {
            outgoingPacket = m_PacketQueue.front();
            m_PacketQueue.pop();
        }

        m_PacketAdditionMutex.Unlock();

        if ( outgoingPacket == 0 ) continue;

        NetworkControl* net = NetworkControl::GetSingleton();

        net->m_ClientAccessMutex.Lock();

        if ( net->m_ClientIds.size() > 0 )
        {
            for ( unsigned int i = 0; i < net->m_ClientIds.size(); ++i )
            {
                if ( outgoingPacket->GetFlag() == Packet::PF_EXCLUDEORIGIN &&
                        outgoingPacket->GetClientPrivateId() == net->m_ClientIds[i].cc_PrivateId )
                {
                    continue;
                }

                else if ( outgoingPacket->GetFlag() == Packet::PF_SPECIFIC  &&
                          outgoingPacket->GetClientPrivateId() != net->m_ClientIds[i].cc_PrivateId )
                {
                    continue;
                }

                Client* client = net->m_PrivateIdClientMap[ net->m_ClientIds[i].cc_PrivateId ];
                if ( client == 0 ) continue;

                Packet* copy = outgoingPacket->GetCopy();

                client->GetSenderObject()->AddToQueue( copy );
            }
        }

        net->m_ClientAccessMutex.Unlock();

        // Cleanup
        delete outgoingPacket;
    }
}
