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

#include "Thread.h"

using namespace ExtExe;

#ifdef _WIN32

Thread::Thread( StartRoutine func, void* arg )
{
	m_ThreadObject = CreateThread( 0, 0, func, arg, 0, 0 );
}

Thread::~Thread( void )
{
	Wait( INFINITE );
	CloseHandle( m_ThreadObject );
}

bool Thread::Wait( unsigned int time )
{
	DWORD result = WaitForSingleObject( m_ThreadObject, (DWORD) time );

	return result == WAIT_OBJECT_0;
}

#endif

#ifdef __linux__

Thread::Thread( StartRoutine func, void* arg )
{
	pthread_attr_t attr;

	sched_param sched;
	sched_setparam( 0, &sched );

	pthread_attr_init( &attr );
	pthread_attr_setschedparam( &attr, &sched );

	int a = pthread_create( &m_ThreadObject, 0, ( void*(*) (void*) )func, arg );
	(void) a;
}

Thread::~Thread( void )
{
	// Do nothing
}

bool Thread::Wait( unsigned int time )
{
	// Time omitted!
	pthread_join( m_ThreadObject, 0 );
	return true;
}

#endif
