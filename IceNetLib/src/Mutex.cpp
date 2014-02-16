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

#include "Mutex.h"

using namespace ExtExe;

#ifdef _WIN32

Mutex::Mutex( void )
{
	InitializeCriticalSection( &m_MutexObject );
}

Mutex::~Mutex( void )
{
	DeleteCriticalSection( &m_MutexObject );
}

void Mutex::Lock( void )
{
	EnterCriticalSection( &m_MutexObject );
}

void Mutex::Unlock( void )
{
	LeaveCriticalSection( &m_MutexObject );
}

InternalMutexObject& Mutex::GetMutexObject( void )
{
	return m_MutexObject;
}

#endif

#ifdef __linux__

Mutex::Mutex( void )
{
	pthread_mutexattr_t attr;

	pthread_mutexattr_init( &attr );
	pthread_mutexattr_setpshared( &attr, PTHREAD_PROCESS_PRIVATE );

	pthread_mutex_init( &m_MutexObject, &attr );
}

Mutex::~Mutex( void )
{
	pthread_mutex_destroy( &m_MutexObject );
}

void Mutex::Lock( void )
{
	pthread_mutex_lock( &m_MutexObject );
}

void Mutex::Unlock( void )
{
	pthread_mutex_unlock( &m_MutexObject );
}

InternalMutexObject& Mutex::GetMutexObject( void )
{
	return m_MutexObject;
}

#endif
