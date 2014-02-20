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

#include "OpCodeHandler.h"

using namespace IceNet;

OpCodeHandler* OpCodeHandler::m_Singleton = 0;

OpCodeHandler::OpCodeHandler( void )
{
	memset( m_OpCodes, 0, sizeof( PACKET_HANDLING_FUNCTION ) * USHRT_MAX );
	memset( m_ClientData, 0, sizeof( void* ) * USHRT_MAX );
}

OpCodeHandler::~OpCodeHandler( void )
{}

OpCodeHandler* OpCodeHandler::GetSingleton( void )
{
	if ( m_Singleton == 0 )
	{
		m_Singleton = new OpCodeHandler();
	}

	return m_Singleton;
}

void OpCodeHandler::LinkOpCodeFunction( OPCODE codeNumber, PACKET_HANDLING_FUNCTION fun, void* clientData )
{
	if ( (unsigned short) ( codeNumber + 256 ) < 256 ) return;

	m_OpCodes[ codeNumber + 256 ] = fun;
	m_ClientData[ codeNumber + 256 ] = clientData;
}

void OpCodeHandler::LinkOpCodeFunctionInternal( OPCODE codeNumber, PACKET_HANDLING_FUNCTION fun )
{
	m_OpCodes[ codeNumber ] = fun;
}

void OpCodeHandler::CallOpCodeFunction( OPCODE codeNumber, Packet* packet )
{
	PACKET_HANDLING_FUNCTION fun = m_OpCodes[ codeNumber ];

	if ( fun != 0 )
	{
		fun( packet, m_ClientData[ codeNumber ] );
	}
}

void OpCodeHandler::SetClientData( OPCODE codeNumber, void* clientData )
{
	if ( (unsigned short) ( codeNumber + 256 ) < 256 ) return;

	m_ClientData[ codeNumber + 256 ] = clientData;
}

void* OpCodeHandler::GetClientData( OPCODE codeNumber )
{
	if ( (unsigned short) ( codeNumber + 256 ) < 256 ) return 0;

	return m_ClientData[ codeNumber + 256 ];
}
