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
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#endif

/*
	EXTEXE Support Code

	class Semaphore:

	A very basic semaphore class that has support for Linux and Win32.
*/

namespace ExtExe
{
#ifdef _WIN32
	typedef HANDLE InternalSemaphoreObject;
#endif

#ifdef __linux__
	typedef sem_t InternalSemaphoreObject;
#endif

	class Semaphore
	{
	public:
		Semaphore( void );
		~Semaphore( void );

	public:
		void Notify( void );
		bool Wait( unsigned int timeInMs );

	private:
		InternalSemaphoreObject m_SemaphoreObject;
	};
};
