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

#include "Packet.h"

using namespace IceNet;

Packet::Packet( void ) :
		m_Offset( sizeof( unsigned short ) * 5 + sizeof( unsigned char ) ),
		m_StreamPosition( m_Offset ),
		m_Flag( PF_FREE ),
		m_MaxSize( INITIAL_ALLOC )
{
	// Allocate INITIAL_ALLOC for m_Data
	m_Data = (char*) malloc( INITIAL_ALLOC );
	memset( m_Data, 0, INITIAL_ALLOC );

	m_Data[0] = 'I';
	m_Data[1] = 'S';

	// Set up pointers to the m_Data string
	m_StreamSize = ( unsigned short* ) ( m_Data + sizeof( unsigned short ) );
	*m_StreamSize = m_Offset - sizeof( unsigned short );

	m_PrivateId = ( CLIENT_ID* ) ( m_Data + sizeof( unsigned short ) * 2 );
	m_PublicId = ( CLIENT_ID* ) ( m_Data + sizeof( unsigned short ) * 3 );

	m_UDPEnabled = ( unsigned char* ) ( m_Data + sizeof( unsigned short ) * 4 );
	m_OpCode = ( unsigned short* ) ( m_Data + sizeof( unsigned short ) * 4 + sizeof( unsigned char ) );
}

Packet::Packet( Packet* pack ) :
		m_Offset( pack->m_Offset ),
		m_StreamPosition( pack->m_StreamPosition ),
		m_Flag( pack->m_Flag ),
		m_MaxSize( pack->m_MaxSize )
{
	m_Data = (char*) malloc( m_MaxSize );
	memcpy( m_Data, pack->m_Data, m_MaxSize );

	// Set up pointers to the m_Data string
	m_StreamSize = ( unsigned short* ) ( m_Data + sizeof( unsigned short ) );

	m_PrivateId = ( CLIENT_ID* ) ( m_Data + sizeof( unsigned short ) * 2 );
	m_PublicId = ( CLIENT_ID* ) ( m_Data + sizeof( unsigned short ) * 3 );

	m_UDPEnabled = ( unsigned char* ) ( m_Data + sizeof( unsigned short ) * 4 );
	m_OpCode = ( unsigned short* ) ( m_Data + sizeof( unsigned short ) * 4 + sizeof( unsigned char ) );
}

Packet::~Packet( void )
{
	free( m_Data );
}

void Packet::SetClientPrivateId( const CLIENT_ID& id )
{
	*m_PrivateId = id;
}

void Packet::SetClientPublicId( const CLIENT_ID& id )
{
	*m_PublicId = id;
}

void Packet::SetUDPEnabled( bool udp )
{
	*m_UDPEnabled = ( unsigned char ) udp;
}

void Packet::SetOpCode( const unsigned short& opCode )
{
	*m_OpCode = opCode + 256;
}

void Packet::SetOpCodeInternal( const unsigned short& opCode )
{
	*m_OpCode = opCode;
}

CLIENT_ID Packet::GetClientPrivateId( void )
{
	return (*m_PrivateId);
}

CLIENT_ID Packet::GetClientPublicId( void )
{
	return (*m_PublicId);
}

bool Packet::GetUDPEnabled( void )
{
	return (*m_UDPEnabled) == 1 ? true : false;
}

unsigned short Packet::GetOpCode( void )
{
	return (*m_OpCode);
}

Packet::PACK_FLAG Packet::GetFlag( void )
{
	return m_Flag;
}

Packet* Packet::GetCopy( void )
{
	Packet* newpack = new Packet( this );

	return newpack;
}

void Packet::AddDataStreaming( POINTER_TO_DATA data, unsigned short sizeOfData )
{
	ResizeCheck( sizeOfData );

	memcpy( &m_Data[ m_StreamPosition ], data, (size_t) sizeOfData );
	m_StreamPosition += sizeOfData;
	(*m_StreamSize) += sizeOfData;
}

char* Packet::RetrieveDataStreaming( unsigned short sizeOfData )
{
	char* object = (char*) ( m_Data + m_StreamPosition );
	m_StreamPosition += sizeOfData;

	return object;
}

void Packet::ResetStreamCounter( void )
{
	m_StreamPosition = m_Offset;
}

void Packet::SetFlag( const PACK_FLAG& flag )
{
	m_Flag = flag;
}

unsigned short Packet::GetSize( void )
{
	return (*m_StreamSize);
}

char* Packet::GetDataStream( void )
{
	return m_Data;
}

void Packet::SetFromDataStream( char* dataStream, unsigned short sizeOfData )
{
	m_Data = (char*) realloc( m_Data, sizeOfData );
	memcpy( m_Data, dataStream, sizeOfData );

	// Set up pointers to the m_Data string
	m_StreamSize = ( unsigned short* ) ( m_Data + sizeof( unsigned short ) );

	m_PrivateId = ( CLIENT_ID* ) ( m_Data + sizeof( unsigned short ) * 2 );
	m_PublicId = ( CLIENT_ID* ) ( m_Data + sizeof( unsigned short ) * 3 );

	m_UDPEnabled = ( unsigned char* ) ( m_Data + sizeof( unsigned short ) * 4 );
	m_OpCode = ( unsigned short* ) ( m_Data + sizeof( unsigned short ) * 4 + sizeof( unsigned char ) );
}

inline void Packet::ResizeCheck( unsigned short size )
{
	bool resize = false;

	// Keep resizing the maxsize until a new object of size ( size ) fits in
	while ( (*m_StreamSize) + size >= m_MaxSize )
	{
		m_MaxSize <<= 1;
		resize = true;
	}

	if ( resize ) 
	{
		m_Data = (char*) realloc( m_Data, (size_t) m_MaxSize );

		// Set up pointers to the m_Data string
		m_StreamSize = ( unsigned short* ) ( m_Data + sizeof( unsigned short ) );

		m_PrivateId = ( CLIENT_ID* ) ( m_Data + sizeof( unsigned short ) * 2 );
		m_PublicId = ( CLIENT_ID* ) ( m_Data + sizeof( unsigned short ) * 3 );

		m_UDPEnabled = ( unsigned char* ) ( m_Data + sizeof( unsigned short ) * 4 );
		m_OpCode = ( unsigned short* ) ( m_Data + sizeof( unsigned short ) * 4 + sizeof( unsigned char ) );
	}
}