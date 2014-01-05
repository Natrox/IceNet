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

#pragma once

#include <vector>

#include <WinSock2.h>
#include <Ws2tcpip.h>

#include "NetworkControl.h"

#include "Client.h"
#include "ClientProxy.h"
#include "ClientContainer.h"
#include "ConnectionInfo.h"
#include "Packet.h"
#include "IceNetTypedefs.h"

/*
	ICECAT Networking		v2.0

	file IceNetClient.h:

	This file is the interface to the client-side functions of IceNet.
	
	To connect to a server running IceNet, use Connect( .. ).
	To disconnect, use Disconnect( void ).

	To link operation codes to functions, use the LinkOpCodeFunction( ... ) function,
	and supply a function with the necessary parameters (found in IceNetTypedefs.h).
	
	Several classes and structs are available;

	Client,
	ClientProxy,
	ClientContainer,
	ConnectionInfo,
	Packet

	** WARNING **
	You MUST set up callbacks before you connect to a server. 
	These callback setting functions are not thread-safe by default!
*/

namespace IceNet
{
	namespace ClientSide
	{
		extern int Connect( PCSTR port, PCSTR ip, unsigned int flags );
		extern void Disconnect( void );

		extern void SendTCP( Packet* packet );
		extern void SendUDP( Packet* packet );

		extern unsigned int HandlePackets( void );

		extern Client* GetLocalClient( void );
		extern ClientProxy* GetRemoteClient( unsigned short publicId );
		extern std::vector< ClientContainer >* GetClientProxyVector( void );

		extern void SetOnConnectionSucceed( VOID_WITH_CLIENT_PARAM fun );
		extern void SetOnConnectionFail( VOID_WITH_CONNECTIONINFO_PARAM fun );
		extern void SetOnDisconnect( VOID_WITH_NO_PARAM fun );

		extern void SetOnAddRemoteClient( VOID_WITH_CLIENTPROXY_PARAM fun );
		extern void SetOnRemoveRemoteClient( VOID_WITH_CLIENTPROXY_PARAM fun );

		extern inline VOID_WITH_CLIENT_PARAM GetOnConnectionSucceed( void );
		extern inline VOID_WITH_CONNECTIONINFO_PARAM GetOnConnectionFail( void );
		extern inline VOID_WITH_NO_PARAM GetOnDisconnect( void );

		extern inline VOID_WITH_CLIENTPROXY_PARAM GetOnAddRemoteClient( void );
		extern inline VOID_WITH_CLIENTPROXY_PARAM GetOnRemoveRemoteClient( void );

		extern void LinkOpCodeFunction( unsigned short codeNumber, PACKET_HANDLING_FUNCTION fun );
	}
}