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

	class Broadcaster:

	This class is used to broadcast packages placed in its queue to every available client depending on the packet flag.
	This class is made to interact with a seperate thread, which will handle all the packages.
*/

namespace IceNet
{
	// Prototypes
	class Packet;

	class Broadcaster
	{
	public:
		Broadcaster( void );
		~Broadcaster( void );

		static Broadcaster* GetSingleton( void );

		// Add a packet to the processing queue.
		void AddToList( Packet* sourcePacket );

		// This function handles all available packets until a stop has been requested.
		void ProcessLoop( void );

		// Handles
		Thread* m_BroadcastThread;
		Event m_StopRequestedEvent;

	private:
		std::queue< Packet* > m_PacketQueue;
		Mutex m_PacketAdditionMutex;

		Semaphore m_PacketQueueSemaphore;
		
	protected:
		static Broadcaster* m_Singleton;

	private:
		Broadcaster( const Broadcaster& brInstance );
		const Broadcaster& operator=( const Broadcaster& brInstance );
	};
};