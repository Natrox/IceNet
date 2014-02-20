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

#include "StandardFunctions.h"
#include "NetworkControl.h"
#include "IceNetClient.h"
#include "OpCodeHandler.h"
#include "Packet.h"

namespace IceNet
{
	void ReceiveID( Packet& pack, void* clientData )
	{
		if ( NetworkControl::GetSingleton()->m_LocalClient->GetPrivateId() != 0 ) return;

		unsigned short publicId = pack.RetrieveDataStreaming< unsigned short >();
		unsigned short privateId = pack.RetrieveDataStreaming< unsigned short >();

		NetworkControl::GetSingleton()->SetPublicId( publicId );
		NetworkControl::GetSingleton()->SetPrivateId( privateId );

		if ( ClientSide::GetOnConnectionSucceed() != 0 ) ClientSide::GetOnConnectionSucceed()( NetworkControl::GetSingleton()->m_LocalClient );
	
		Packet* pack2 = new Packet();

		pack2->SetOpCodeInternal( 0 );
		pack2->SetClientPrivateId( privateId );

		ClientSide::SendUDP( pack2 );
	}

	void AddRemoteClient( Packet& pack, void* clientData )
	{
		unsigned short publicId = pack.RetrieveDataStreaming< unsigned short >();

		ClientProxy* client = NetworkControl::GetSingleton()->AddClientProxy( publicId );

		if ( ClientSide::GetOnAddRemoteClient() != 0 ) ClientSide::GetOnAddRemoteClient()( client );
	}

	void RemoveRemoteClient( Packet& pack, void* clientData )
	{
		unsigned short publicId = pack.RetrieveDataStreaming< unsigned short >();

		ClientProxy* client = NetworkControl::GetSingleton()->m_PublicIdClientProxyMap[ publicId ];
		
		if ( client == 0 )
		{
			return;
		}

		if ( ClientSide::GetOnRemoveRemoteClient() != 0 ) ClientSide::GetOnRemoveRemoteClient()( client );
		NetworkControl::GetSingleton()->RemoveClientProxy( publicId );
	}

	void LinkClientFunctions( void )
	{
		OpCodeHandler::GetSingleton()->LinkOpCodeFunctionInternal( OpCodeHandler::GET_ID, ReceiveID );
		OpCodeHandler::GetSingleton()->LinkOpCodeFunctionInternal( OpCodeHandler::ADD_CLIENT, AddRemoteClient );
		OpCodeHandler::GetSingleton()->LinkOpCodeFunctionInternal( OpCodeHandler::REMOVE_CLIENT, RemoveRemoteClient );
	}

	void LinkServerFunctions( void )
	{}
}