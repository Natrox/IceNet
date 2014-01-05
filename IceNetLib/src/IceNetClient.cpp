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

#include "IceNetClient.h"
#include "OpCodeHandler.h"

#include "PacketSender.h"
#include "PacketHandler.h"

namespace IceNet
{
	namespace ClientSide
	{
		// Callback functions
		VOID_WITH_CLIENT_PARAM g_OnSucceed = 0;
		VOID_WITH_CONNECTIONINFO_PARAM g_OnFail = 0;
		VOID_WITH_NO_PARAM g_OnDisconnect = 0;

		VOID_WITH_CLIENTPROXY_PARAM g_AddRemote = 0;
		VOID_WITH_CLIENTPROXY_PARAM g_RemoveRemote = 0;

		int Connect( PCSTR port, PCSTR ip, unsigned int flags = NetworkControl::PROTOCOL_UDP )
		{
			NetworkControl::ErrorCodes error = NetworkControl::InitializeClient( port, ip, (NetworkControl::Flags) flags );
		
			ConnectionInfo cinfo = { ip, port };
			if ( error != NetworkControl::ERROR_NONE && g_OnFail != 0 ) g_OnFail( cinfo );
		
			return (int) error;
		}

		void Disconnect( void )
		{
			if ( NetworkControl::GetSingleton()->m_LocalClient != 0 ) SetEvent( NetworkControl::GetSingleton()->m_LocalClient->m_StopEvent );
			WaitForSingleObject( NetworkControl::GetSingleton()->m_NetworkThreadHandle, INFINITE );

			NetworkControl::Deinitialize();

			WaitForSingleObject( NetworkControl::GetSingleton()->m_RecycleConnection, INFINITE );

			if ( g_OnDisconnect != 0 ) g_OnDisconnect();
		}

		void SendTCP( Packet* packet )
		{
			packet->SetClientPrivateId( NetworkControl::GetSingleton()->m_LocalClient->m_PrivateId );
		
			packet->SetUDPEnabled( false );
			NetworkControl::GetSingleton()->m_LocalClient->GetSenderObject()->AddToQueue( packet );
		}

		void SendUDP( Packet* packet ) 
		{
			packet->SetClientPrivateId( NetworkControl::GetSingleton()->m_LocalClient->m_PrivateId );

			packet->SetUDPEnabled( true );
			NetworkControl::GetSingleton()->m_LocalClient->GetSenderObject()->AddToQueue( packet );
		}

		unsigned int HandlePackets( void )
		{
			if ( NetworkControl::GetSingleton()->GetPacketHandler() == 0 ) return 0;

			return NetworkControl::GetSingleton()->GetPacketHandler()->HandlePackets();
		}

		Client* GetLocalClient( void )
		{
			return NetworkControl::GetSingleton()->m_LocalClient;
		}

		ClientProxy* GetRemoteClient( unsigned short publicId )
		{
			return NetworkControl::GetSingleton()->m_PublicIdClientProxyMap[ publicId ];
		}

		std::vector< ClientContainer >* GetClientProxyVector( void )
		{
			return &( NetworkControl::GetSingleton()->m_ClientProxyIds );
		}

		void SetOnConnectionSucceed( VOID_WITH_CLIENT_PARAM fun )
		{
			g_OnSucceed = fun;
		}

		void SetOnConnectionFail( VOID_WITH_CONNECTIONINFO_PARAM fun )
		{
			g_OnFail = fun;
		}

		void SetOnDisconnect( VOID_WITH_NO_PARAM fun )
		{
			g_OnDisconnect = fun;
		}

		void SetOnAddRemoteClient( VOID_WITH_CLIENTPROXY_PARAM fun )
		{
			g_AddRemote = fun;
		}

		void SetOnRemoveRemoteClient( VOID_WITH_CLIENTPROXY_PARAM fun )
		{
			g_RemoveRemote = fun;
		}

		inline VOID_WITH_CLIENT_PARAM GetOnConnectionSucceed( void )
		{
			return g_OnSucceed;
		}

		inline VOID_WITH_CONNECTIONINFO_PARAM GetOnConnectionFail( void )
		{
			return g_OnFail;
		}

		inline VOID_WITH_NO_PARAM GetOnDisconnect( void )
		{
			return g_OnDisconnect;
		}

		inline VOID_WITH_CLIENTPROXY_PARAM GetOnAddRemoteClient( void )
		{
			return g_AddRemote;
		}

		inline VOID_WITH_CLIENTPROXY_PARAM GetOnRemoveRemoteClient( void )
		{
			return g_RemoveRemote;
		}

		void LinkOpCodeFunction( unsigned short codeNumber, PACKET_HANDLING_FUNCTION fun )
		{
			OpCodeHandler::GetSingleton()->LinkOpCodeFunction( (IceNet::OPCODE) codeNumber, fun );	
		}
	}
}