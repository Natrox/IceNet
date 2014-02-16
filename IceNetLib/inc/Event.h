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

#include <errno.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __linux__
#include "Mutex.h"
#include <pthread.h>
#endif

/*
	EXTEXE Support Code

	class Event:

	A very basic event class that has support for Linux and Win32.
*/

namespace ExtExe
{
#ifdef _WIN32
	typedef HANDLE InternalEventObject;
#endif

#ifdef __linux__
	struct InternalEventObject
	{
		Mutex ieo_Mutex;
		pthread_cond_t ieo_Cond;
		int ieo_ManualReset;
		bool ieo_Predicate;
	};
#endif

	class Event
	{
	public:
		Event( int manualReset );
		~Event( void );

	public:
		void Set( void );
		void Reset( void );
		bool Wait( unsigned int timeInMs );

	private:
		InternalEventObject m_EventObject;
	};
};
