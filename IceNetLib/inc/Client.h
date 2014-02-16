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

#include "Platforms.h"
#include "Threading.h"

#include <string>

/*
	ICECAT Networking		v2.0

	class Client:

	This class is used to store client information.

	Remarkss:
	Private ID is only known to the client it belongs to and the server. It is used for data transfer.
	Public ID is known to anyone. It is used to identify a client from another client.
*/

namespace IceNet
{
	// Typedefs
	typedef unsigned short CLIENT_ID;
	typedef void* PTR_ANYTHING;

	// Prototypes
	class PacketSender;
	class PacketReceiver;
	class PacketHandler;
	class Packet;

	namespace ClientSide
	{
		void SendTCP( Packet* packet );
		void SendUDP( Packet* packet );
	};

	namespace ServerSide
	{
		void SendTCP( Packet* packet, bool deletePacket );
		void SendUDP( Packet* packet, bool deletePacket );
	};

	class Client
	{
	public:
		Client( CLIENT_ID publicId, CLIENT_ID privateId = 0, bool local = false, SOCKET socket = 0 );
		~Client( void );

		// This function wakes up and kills the client if certain conditions are met
		static THREAD_FUNC KeepAlive( void* client );
		SOCKET GetSocket( void );

		// This allows you to store and retrieve any kind of object
		void SetAssociatedObject( PTR_ANYTHING object );
		template < typename T > T* GetAssociatedObject( void ) { return (T*) m_AssociatedObject; }

		// We need to know the UDP origin to send UDP data back, so we'll need these functions
		sockaddr GetUDPOrigin( void );
		bool CompareUDPOrigin( const sockaddr& addr );
		void SetStop( void );

		// Check if the client is local, e.g. if the client is created on a client-side configuration.
		bool IsLocal( void );

		// IP address
		std::string GetIPAddress( void );

		// Getters
		Event& GetStopEvent( void );
		CLIENT_ID GetPublicId( void );
		CLIENT_ID GetPrivateId( void );

	private:
		Event m_StopEvent;
		Thread* m_Thread;

		SOCKET m_SocketTCP;
		CLIENT_ID m_PublicId;
		CLIENT_ID m_PrivateId;

		sockaddr m_UDPOrigin;
		bool m_UDPInitialized;
		std::string m_IP;

		PacketSender* m_SenderObject;
		PacketReceiver* m_ReceiverObject;
		PacketHandler* m_HandlerObject;

		mutable PTR_ANYTHING m_AssociatedObject;
		Mutex m_AccessMutex;

		bool m_Local;

	private:
		void SetUDPOrigin( const sockaddr& addr );
		void SetSocket( const SOCKET& socket );

		// These are the various facilities for handling incoming/outgoing packets.
		PacketSender* GetSenderObject( void );
		PacketReceiver* GetReceiverObject( void );
		PacketHandler* GetHandlerObject( void );

	protected:
		// These classes and functions need full access to this class.
		friend class PacketSender;
		friend class PacketReceiver;
		friend class PacketHandler;
		friend class UDPReceiver;
		friend class NetworkControl;
		friend class Broadcaster;

		friend void ClientSide::SendTCP( Packet* packet );
		friend void ClientSide::SendUDP( Packet* packet );

		friend void ServerSide::SendTCP( Packet* packet, bool deletePacket );
		friend void ServerSide::SendUDP( Packet* packet, bool deletePacket );

		friend THREAD_FUNC ClientEntry( void* ptr );
		friend THREAD_FUNC ListenerEntry( void* ptr );
	};
};
