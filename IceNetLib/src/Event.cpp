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

#include "Event.h"

using namespace ExtExe;

#ifdef _WIN32

Event::Event( int manualReset )
{
	m_EventObject = CreateEvent( 0, manualReset, 0, 0 );
}

Event::~Event( void )
{
	CloseHandle( m_EventObject );
}

void Event::Set( void )
{
	SetEvent( m_EventObject );
}

void Event::Reset( void )
{
	ResetEvent( m_EventObject );
}

bool Event::Wait( unsigned int time )
{
	DWORD result = WaitForSingleObject( m_EventObject, (DWORD) time );

	return result == WAIT_OBJECT_0;
}

#endif

#ifdef __linux__

#include <stdio.h>
#include <sys/time.h>

Event::Event( int manualReset )
{
	pthread_cond_init( &m_EventObject.ieo_Cond, 0 );
	m_EventObject.ieo_ManualReset = manualReset;
	m_EventObject.ieo_Predicate = false;
}

Event::~Event( void )
{
	pthread_cond_destroy( &m_EventObject.ieo_Cond );
}

void Event::Set( void )
{
	m_EventObject.ieo_Mutex.Lock();
	pthread_cond_signal( &m_EventObject.ieo_Cond );
	m_EventObject.ieo_Predicate = true;
	m_EventObject.ieo_Mutex.Unlock();
}

void Event::Reset( void )
{
	m_EventObject.ieo_Mutex.Lock();
	m_EventObject.ieo_Predicate = false;
	m_EventObject.ieo_Mutex.Unlock();
}

bool Event::Wait( unsigned int timeInMs )
{
	m_EventObject.ieo_Mutex.Lock();

	struct timeval tv;
	struct timespec ts;

	gettimeofday( &tv, NULL );
	ts.tv_sec = time( NULL ) + timeInMs / 1000;
	ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * ( timeInMs % 1000 );
	ts.tv_sec += ts.tv_nsec / ( 1000 * 1000 * 1000 );
	ts.tv_nsec %= ( 1000 * 1000 * 1000 );

	pthread_mutex_t& mutex = m_EventObject.ieo_Mutex.GetMutexObject();

	bool succeed = false;

	if ( timeInMs != 0 )
	{
		pthread_cond_timedwait( &m_EventObject.ieo_Cond, &mutex, &ts );
		succeed = !( errno == ETIMEDOUT );
	}

	else
	{
		succeed = m_EventObject.ieo_Predicate;
	}

	if ( succeed && m_EventObject.ieo_ManualReset == 0 )
	{
		pthread_cond_signal( &m_EventObject.ieo_Cond );
		m_EventObject.ieo_Predicate = true;
	}

	m_EventObject.ieo_Mutex.Unlock();

	return succeed;
}

#endif
