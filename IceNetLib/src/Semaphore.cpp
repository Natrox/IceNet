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

#include "Semaphore.h"

using namespace ExtExe;

#ifdef _WIN32

Semaphore::Semaphore( void )
{
	m_SemaphoreObject = CreateSemaphore( 0, 0, LONG_MAX, 0 );
}

Semaphore::~Semaphore( void )
{
	CloseHandle( m_SemaphoreObject );
}

void Semaphore::Notify( void )
{
	ReleaseSemaphore( m_SemaphoreObject, 1, 0 );
}

bool Semaphore::Wait( unsigned int time )
{
	DWORD result = WaitForSingleObject( m_SemaphoreObject, (DWORD) time );

	return result == WAIT_OBJECT_0;
}

#endif

#ifdef __linux__

#include <sstream>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <sys/time.h>

#include "Mutex.h"

Semaphore::Semaphore( void )
{
	int a = sem_init( &m_SemaphoreObject, 0, 0 );

	if ( a != 0 ) std::cout << "Created Semaphore " << strerror( a ) << "\n";
}

Semaphore::~Semaphore( void )
{
	sem_destroy( &m_SemaphoreObject );
}

void Semaphore::Notify( void )
{
	sem_post( &m_SemaphoreObject );
}

bool Semaphore::Wait( unsigned int timeInMs )
{
    struct timeval tv;
    struct timespec ts;

    gettimeofday( &tv, NULL );
    ts.tv_sec = time( NULL ) + timeInMs / 1000;
    ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * ( timeInMs % 1000 );
    ts.tv_sec += ts.tv_nsec / ( 1000 * 1000 * 1000 );
    ts.tv_nsec %= ( 1000 * 1000 * 1000 );

	sem_timedwait( &m_SemaphoreObject, &ts );
	return !( errno == ETIMEDOUT );
}

#endif
