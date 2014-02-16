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

	THREAD_FUNC ServerEntry( void* ptr )
	{
		LinkServerFunctions();

		// Create a new thread for the broadcaster
		Broadcaster::GetSingleton()->m_BroadcastThread = new Thread( BroadcastEntry, 0 );

		// Create a new thread for the listener
		Thread lsThr( ListenerEntry, 0 );

		// Create a new thread for the UDP receiver
		Thread urThr( UDPReceiverEntry, 0 );

		// Wait for a stop
		while ( !NetworkControl::GetSingleton()->m_StopRequestedEvent.Wait( INFINITE ) );
		Broadcaster::GetSingleton()->m_StopRequestedEvent.Set();
		Broadcaster::GetSingleton()->AddToList( 0 );

		// Wait for everything to close down
		Broadcaster::GetSingleton()->m_BroadcastThread->Wait( INFINITE );
		delete Broadcaster::GetSingleton()->m_BroadcastThread;

		lsThr.Wait( INFINITE );
		urThr.Wait( INFINITE );

		return 1;
	}

	// Default entry point for the client configuration

	THREAD_FUNC DisconnectAsync( void* ptr )
	{
		ClientSide::Disconnect();
		return 1;
	}

	THREAD_FUNC ClientEntry( void* ptr )
	{
		LinkClientFunctions();

		// Connect and create a local client object.
		if ( NetworkControl::GetSingleton()->ConnectToHost() == -1 )
		{
			ConnectionInfo cinfoReplc = { 0, 0 };
			if ( ClientSide::GetOnConnectionFail() != 0 ) ClientSide::GetOnConnectionFail()( cinfoReplc );

			NetworkControl::GetSingleton()->m_StopRequestedEvent.Set();
			closesocket( NetworkControl::GetSingleton()->m_SocketUDP );

			// Nasty hack, will fix later
			Thread thread( DisconnectAsync, 0 );

			return 1;
		}

		NetworkControl::GetSingleton()->m_LocalClient = new Client( 256, 0, true, NetworkControl::GetSingleton()->m_SocketTCP );

		// Create the UDP listening thread
		Thread urThr( UDPReceiverEntry, 0 );

		// Wait for everything to close down.
		NetworkControl::GetSingleton()->m_LocalClient->m_StopEvent.Wait( INFINITE );
		NetworkControl::GetSingleton()->m_StopRequestedEvent.Set();
		closesocket( NetworkControl::GetSingleton()->m_SocketUDP );

		urThr.Wait( INFINITE );

		// Call the disconnect callback if it exists.
		if ( ClientSide::GetOnDisconnect() != 0 ) ClientSide::GetOnDisconnect()();

		return 1;
	}
};
