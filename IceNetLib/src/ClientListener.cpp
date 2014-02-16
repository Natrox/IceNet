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

#include "Modules.h"
#include "OpCodeHandler.h"
#include "NetworkControl.h"
#include "PacketSender.h"
#include "Packet.h"
#include "Client.h"

#include "IceNetServer.h"
#include "IceNetTypedefs.h"

#include <time.h>

namespace IceNet
{
	unsigned short RandomID( void )
	{
		unsigned short id = 0;

		while ( 1 )
		{
			id = rand() & USHRT_MAX;
			if ( id <= 256 ) continue;

			break;
		}

		return id;
	}

	THREAD_FUNC ListenerEntry( void* ptr )
	{
		srand( (unsigned int) time( 0 ) );
		rand();

		while ( true )
		{
			// Poll for stop condition
			if ( NetworkControl::GetSingleton()->m_StopRequestedEvent.Wait( 0 ) )
			{
				break;
			}

			sockaddr tcpOrigin;

#ifdef _WIN32
			int size = (int) sizeof( sockaddr );
#endif

#ifdef __linux__
            socklen_t size = (socklen_t) sizeof( sockaddr );
#endif
			// Accept a new connection (blocking)
			SOCKET clientSock = accept( NetworkControl::GetSingleton()->m_SocketTCP, 0, 0 );

			// Get some data from the peer
			getpeername( clientSock, &tcpOrigin, &size );

			// If something went wrong, simply continue;
			if ( clientSock == -1 )
			{
				continue;
			}

			unsigned short publicId = RandomID();
			unsigned short privateId = RandomID();

			while ( NetworkControl::GetSingleton()->m_PublicIdClientMap[ publicId ] != 0 ) publicId = RandomID();
			while ( NetworkControl::GetSingleton()->m_PrivateIdClientMap[ privateId ] != 0 ) privateId = RandomID();

			// Create a new client
			Client* newClientObj = NetworkControl::GetSingleton()->AddClient( publicId, privateId, false, clientSock );

			// Copy the origin data
			newClientObj->m_IP = inet_ntoa( ( (sockaddr_in*) &tcpOrigin )->sin_addr );

			// Call the callback if it exists
			VOID_WITH_CLIENT_PARAM fun = ServerSide::GetOnAddClient();
			if ( fun != 0 ) fun( newClientObj );

			// Ship the new client's IDs back.
			Packet* sendidpack = new Packet();

			sendidpack->SetOpCodeInternal( OpCodeHandler::GET_ID );
			sendidpack->AddDataStreaming<unsigned short>( (unsigned short) newClientObj->m_PublicId );
			sendidpack->AddDataStreaming<unsigned short>( (unsigned short) newClientObj->m_PrivateId );
			sendidpack->SetUDPEnabled( false );

			newClientObj->GetSenderObject()->AddToQueue( sendidpack );

			// Broadcast the public ID to all other clients.
			Packet* broadcast = new Packet();

			broadcast->SetOpCodeInternal( OpCodeHandler::ADD_CLIENT );
			broadcast->AddDataStreaming<unsigned short>( (unsigned short) newClientObj->m_PublicId );
			broadcast->SetClientPrivateId( newClientObj->m_PrivateId );
			broadcast->SetFlag( Packet::PF_EXCLUDEORIGIN );
			sendidpack->SetUDPEnabled( false );

			NetworkControl::GetSingleton()->BroadcastToAll( broadcast );

			if ( NetworkControl::GetSingleton()->m_ClientIds.size() > 0 )
			{
				for ( unsigned int i = 0; i < NetworkControl::GetSingleton()->m_ClientIds.size(); i++ )
				{
					if ( NetworkControl::GetSingleton()->m_ClientIds[i].cc_PublicId != publicId )
					{
						Packet* pack = new Packet();

						pack->SetOpCodeInternal( OpCodeHandler::ADD_CLIENT );
						pack->AddDataStreaming<unsigned short>( (unsigned short) NetworkControl::GetSingleton()->m_ClientIds[i].cc_PublicId );

						newClientObj->GetSenderObject()->AddToQueue( pack );
					}
				}
			}
		}

		return 1;
	}
};
