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
#include "Modules.h"
#include "Broadcaster.h"
#include "Client.h"
#include "IceNetClient.h"

#include "StandardFunctions.h"

using namespace IceNet;

namespace IceNet
{
	// Default entry point for the server configuration

	DWORD WINAPI ServerEntry( void* ptr )
	{
		LinkServerFunctions();

		// Create a new thread for the broadcaster
		HANDLE bcThr = CreateThread( NULL, NULL, BroadcastEntry, NULL, NULL, NULL );
		Broadcaster::GetSingleton()->m_BroadcastThreadHandle = bcThr;

		// Create a new thread for the listener
		HANDLE lsThr = CreateThread( NULL, NULL, ListenerEntry, NULL, NULL, NULL );

		// Create a new thread for the UDP receiver
		HANDLE urThr = CreateThread( NULL, NULL, UDPReceiverEntry, NULL, NULL, NULL );

		// Wait for a stop
		WaitForSingleObject( NetworkControl::GetSingleton()->m_StopRequestedEvent, INFINITE );
		SetEvent( Broadcaster::GetSingleton()->m_StopRequestedEvent );

		// Wait for everything to close down
		WaitForSingleObject( bcThr, INFINITE );
		CloseHandle( bcThr );

		WaitForSingleObject( lsThr, INFINITE );
		CloseHandle( lsThr );

		WaitForSingleObject( urThr, INFINITE );
		CloseHandle( urThr );

		return 1;
	}

	// Default entry point for the client configuration

	DWORD WINAPI DisconnectAsync( void* ptr )
	{
		ClientSide::Disconnect();
		return 1;
	}

	DWORD WINAPI ClientEntry( void* ptr )
	{
		LinkClientFunctions();

		// Connect and create a local client object.
		if ( NetworkControl::GetSingleton()->ConnectToHost() == -1 )
		{
			ConnectionInfo cinfoReplc = { 0, 0 };
			if ( ClientSide::GetOnConnectionFail() != 0 ) ClientSide::GetOnConnectionFail()( cinfoReplc );

			SetEvent( NetworkControl::GetSingleton()->m_StopRequestedEvent );
			closesocket( NetworkControl::GetSingleton()->m_SocketUDP );

			// Nasty hack, will fix later
			CreateThread( 0, 0, DisconnectAsync, 0, 0, 0 );

			return 1;
		}

		NetworkControl::GetSingleton()->m_LocalClient = new Client( 256, 0, true, NetworkControl::GetSingleton()->m_SocketTCP );

		// Create the UDP listening thread
		HANDLE urThr = CreateThread( NULL, NULL, UDPReceiverEntry, NULL, NULL, NULL );

		// Wait for everything to close down.
		WaitForSingleObject( NetworkControl::GetSingleton()->m_LocalClient->m_StopEvent, INFINITE );
		SetEvent( NetworkControl::GetSingleton()->m_StopRequestedEvent );
		closesocket( NetworkControl::GetSingleton()->m_SocketUDP );

		WaitForSingleObject( urThr, INFINITE );
		CloseHandle( urThr );

		// Call the disconnect callback if it exists.
		if ( ClientSide::GetOnDisconnect() != 0 ) ClientSide::GetOnDisconnect()();

		return 1;
	}
};