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

#include <WinSock2.h>
#include <Ws2tcpip.h>

#include <queue>

/*
	ICECAT Networking		v2.0

	class PacketHandler:

	This class handles the packets given to it.
*/

namespace IceNet
{
	class Packet;
	class Client;

	class PacketHandler
	{
	public:
		PacketHandler( Client* parentClient );
		~PacketHandler( void );

		static DWORD WINAPI PacketHandlerLoop( void* packetHandler );
		unsigned int HandlePackets( void );
		void AddToQueue( Packet* packet );

		HANDLE m_ThreadHandle;

	private:
		Client* m_ParentClient;
		std::queue< Packet* > m_PacketQueue;

		HANDLE m_PacketQueueSemaphore;

		CRITICAL_SECTION m_PacketAdditionCSec;
	};
};