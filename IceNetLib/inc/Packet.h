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

/*
	ICECAT Networking		v2.0

	class Packet:

	This class is a container for data to be sent over the network.
*/

/*
	PACKET STRUCTURE (11b (on x86)):

	 ushort		  | ushort	    | ushort     | ushort     | uchar | ushort | char* |
	--------------|-------------|------------|------------|-------|--------|-------|
	 MAGIC NUMBER | PACKET SIZE | PRIVATE ID | PUBLIC ID  | UDP   | OPCODE | DATA  |
*/

#define INITIAL_ALLOC 256
#define MAGIC_CHAR_A "I"
#define MAGIC_CHAR_B "S"

typedef void* POINTER_TO_DATA;

namespace IceNet
{
	typedef unsigned short CLIENT_ID; 

	class Client;

	class Packet
	{
	public:
		// Flags for the broadcaster class
		enum PACK_FLAG
		{
			PF_FREE, // Packet will be sent to all.
			PF_SPECIFIC, // Packet will only be sent to associated socket (useful?)
			PF_EXCLUDEORIGIN // Packets will be sent to all excluding the client the packet originated from.
		};

		Packet( void );
		Packet( Packet* pack );
		~Packet( void );

		// Set functions
		void SetClientPrivateId( const CLIENT_ID& id );
		void SetClientPublicId( const CLIENT_ID& id );
		void SetUDPEnabled( bool udp );
		void SetOpCode( const unsigned short& opCode );

		// Get functions
		CLIENT_ID GetClientPrivateId( void );
		CLIENT_ID GetClientPublicId( void );
		bool GetUDPEnabled( void );
		unsigned short GetOpCode( void );
		PACK_FLAG GetFlag( void );
		Packet* GetCopy( void );

		template<typename T>
		struct is_pointer { static const bool value = false; };

		template<typename T>
		struct is_pointer<T*> { static const bool value = true; };

		// Add data in a streaming manner (position in data changes with each added object)
		template < typename T > void AddDataStreaming( T data )
		{
			ResizeCheck( sizeof( T ) );

			// Copies the data to the m_Data string
			memcpy( &m_Data[ m_StreamPosition ], (const void*) &data, sizeof( T ) );

			m_StreamPosition += sizeof( T );
			(*m_StreamSize) += sizeof( T );
		}

		// Add data of any size
		void AddDataStreaming( POINTER_TO_DATA data, unsigned short sizeOfData );

		// Retrieve data in a streaming manner
		template < typename T > T RetrieveDataStreaming( void )
		{
			// Return data from the m_Data string as object T
			T object;

			memcpy( &object, &m_Data[ m_StreamPosition ], sizeof( T ) );
			m_StreamPosition += sizeof( T );

			return object;
		}

		// Retrieve data of any size
		char* RetrieveDataStreaming( unsigned short sizeOfData );

		// Reset the stream counters. Use with care
		void ResetStreamCounter( void );

		// Sets a flag for this packet

		void SetFlag( const PACK_FLAG& flag );

		// Returns the size (minus *m_StreamSize)
		unsigned short GetSize( void );

		// Returns a string so it can be sent over the network
		char* GetDataStream( void );

		// Construct the package from a string
		void SetFromDataStream( char* dataStream, unsigned short sizeOfData ); 

	private:
		void SetOpCodeInternal( const unsigned short& opCode );

		// Checks if the current size is adequate for new objects
		inline void ResizeCheck( unsigned short size );

		const unsigned short m_Offset;
		unsigned short m_MaxSize;

		unsigned short m_StreamPosition;
		unsigned short* m_StreamSize;

		CLIENT_ID* m_PrivateId;
		CLIENT_ID* m_PublicId;

		unsigned char* m_UDPEnabled;
		unsigned short* m_OpCode;

		char* m_Data;
		PACK_FLAG m_Flag;

		// Assignment operator overloading, to fix a warning.
		const Packet& operator=( const Packet& packInstance );

		friend DWORD WINAPI ListenerEntry( void* ptr );
		friend class Client;
		friend void ReceiveID( Packet* pack );
	};
};