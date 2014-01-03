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

#include "IceNetServer.h"

#include "NetworkControl.h"
#include "OpCodeHandler.h"

#include "PacketSender.h"

namespace IceNet
{
	namespace ServerSide
	{
		// Callback functions
		VOID_WITH_CLIENT_PARAM g_Add = 0;
		VOID_WITH_CLIENT_PARAM g_Remove = 0;

		int Initialize( PCSTR port )
		{
			NetworkControl::SV_ERRORCODE error = NetworkControl::InitializeServer( port, NetworkControl::PACK_UDP );
		
			return (int) error;
		}

		void Deinitialize( void )
		{
			NetworkControl::Deinitialize();
		}

		void SendTCP( Packet* packet, bool deletePacket )
		{
			packet->SetUDPEnabled( false );

			Client* client = NetworkControl::GetSingleton()->m_PrivateIdClientMap[ packet->GetClientPrivateId() ];

			if ( client != 0 )
			{
				client->GetSenderObject()->AddToQueue( packet );
			}

			else if ( deletePacket )
			{
				delete packet;
			}
		}

		void SendUDP( Packet* packet, bool deletePacket )
		{
			packet->SetUDPEnabled( true );

			Client* client = NetworkControl::GetSingleton()->m_PrivateIdClientMap[ packet->GetClientPrivateId() ];

			if ( client != 0 )
			{
				client->GetSenderObject()->AddToQueue( packet );
			}

			else if ( deletePacket )
			{
				delete packet;
			}
		}

		void Broadcast( Packet* packet )
		{
			NetworkControl::GetSingleton()->BroadcastToAll( packet );
		}

		Client* GetClientPublic( unsigned short publicId )
		{
			return NetworkControl::GetSingleton()->m_PublicIdClientMap[ publicId ];
		}

		Client* GetClientPrivate( unsigned short privateId )
		{
			return NetworkControl::GetSingleton()->m_PrivateIdClientMap[ privateId ];
		}

		std::vector< ClientContainer >* GetClientVector( void )
		{
			return &( NetworkControl::GetSingleton()->m_ClientIds );
		}

		void SetOnAddClient( VOID_WITH_CLIENT_PARAM fun )
		{
			g_Add = fun;
		}

		void SetOnRemoveClient( VOID_WITH_CLIENT_PARAM fun )
		{
			g_Remove = fun;
		}
	
		inline VOID_WITH_CLIENT_PARAM GetOnAddClient( void )
		{
			return g_Add;
		}

		inline VOID_WITH_CLIENT_PARAM GetOnRemoveClient( void )
		{
			return g_Remove;
		}

		void LinkOpCodeFunction( unsigned short codeNumber, PACKET_HANDLING_FUNCTION fun )
		{
			OpCodeHandler::GetSingleton()->LinkOpCodeFunction( (IceNet::OPCODE) codeNumber, fun );	
		}
	}
}