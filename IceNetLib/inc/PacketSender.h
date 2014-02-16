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

#include <queue>

/*
	ICECAT Networking		v2.0

	class PacketSender:

	This class sends packets using a queue.
*/

namespace IceNet
{
	class Packet;
	class Client;

	class PacketSender
	{
	public:
		PacketSender( Client* parentClient );
		~PacketSender( void );

		static THREAD_FUNC PacketSenderLoop( void* packetSender );
		void AddToQueue( Packet* packet );

		Thread* m_Thread;

	private:
		Client* m_ParentClient;
		std::queue< Packet* > m_PacketQueue;

		Semaphore m_PacketQueueSemaphore;
		Mutex m_PacketAdditionMutex;
	};
};
