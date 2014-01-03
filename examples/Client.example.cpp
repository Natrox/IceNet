// The following is psuedo-code

#include <Windows.h>

#include "IceNetClient.h"
using namespace IceNet::ClientSide;

ThreadSafeBool g_Confirmed( false );

void PlayBeepSound( Packet* packet )
{
	int hz = packet->RetrieveDataStreaming< int >();
	int ms = packet->RetrieveDataStreaming< int >();

	Beep( hz, ms );
}

void OnRemoteClient( ClientProxy* proxy )
{
	printf( "A client with public ID %d has connected.\n", proxy->m_PublicId );
}

void OnConnect( Client* myClient )
{
	g_Confirmed = true;
}

int main( void )
{
	LinkOpCodeFunction( 1, PlayBeepSound );

	SetOnAddRemoteClient( OnRemoteClient );
	SetOnConnectionSucceed( OnConnect );

	Connect( "9216", "127.0.0.1" );

	WaitFor( g_Confirmed );

	while ( true )
	{
		Packet* newPacket = new Packet();

		// Hello world packet.
		newPacket->SetOpCode( 2 );
		newPacket->SetClientPublicId( GetLocalClient()->m_PublicId );
		newPacket->SetClientPrivateId( GetLocalClient()->m_PrivateId );

		SendTCP( newPacket );

		Sleep( 5000 );
	}
}