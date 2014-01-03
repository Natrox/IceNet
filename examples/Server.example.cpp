// The following is psuedo-code

#include <Windows.h>

#include "IceNetServer.h"
using namespace IceNet;
using namespace IceNet::ServerSide;

void OnJoin( Client* cl )
{
	printf( "Client has joined the server! ( ID: Public %d, Private %d )\n", cl->m_PublicId, cl->m_PrivateId );
}

void OnPart( Client* cl )
{
	printf( "Client has left the server! ( ID: Public %d, Private %d )\n", cl->m_PublicId, cl->m_PrivateId );
}

void ClientMessage( Packet* packet )
{
	printf( "Hello world from client. ( ID: Public %d, Private %d )\n", packet->GetClientPublicId(), packet->GetClientPrivateId() );
}

int main( void )
{
	LinkOpCodeFunction( 2, ClientMessage );

	SetOnAddClient( OnJoin );
	SetOnRemoveClient( OnPart );

	Initialize( "9216" );

	while ( true )
	{
		Packet* newPacket = new Packet();

		// Beep packet
		newPacket->SetOpCode( 1 );
		newPacket->AddDataStreaming<int>( rand() % 3200 + 250 );
		newPacket->AddDataStreaming<int>( rand() % 1000 + 250 );

		Broadcast( newPacket );

		Sleep( 1000 );
	}
}
