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

/*
	ICECAT Networking		v2.0

	class OpCodeHandler:

	This class is used to handle opcodes.
*/

namespace IceNet
{
	class Packet;

	typedef unsigned short OPCODE;
	typedef void ( *PACKET_HANDLING_FUNCTION )( Packet* );

	void LinkClientFunctions( void );

	class OpCodeHandler
	{
	public:
		// These enums are used to specify internally linked functions.
		enum CL_INTERNAL
		{
			GET_ID = 1,
			ADD_CLIENT,
			REMOVE_CLIENT
		};

		enum SV_INTERNAL
		{
			SET_UDP = 1
		};

		OpCodeHandler( void );
		~OpCodeHandler( void );

		static OpCodeHandler* GetSingleton( void );

		void LinkOpCodeFunction( OPCODE codeNumber, PACKET_HANDLING_FUNCTION fun );
		PACKET_HANDLING_FUNCTION GetOpCodeFunction( OPCODE codeNumber );

	private:
		void LinkOpCodeFunctionInternal( OPCODE codeNumber, PACKET_HANDLING_FUNCTION fun );
		PACKET_HANDLING_FUNCTION m_OpCodes[ USHRT_MAX ]; 

		static OpCodeHandler* m_Singleton;

		OpCodeHandler( const OpCodeHandler& opInstance );
		const OpCodeHandler& operator=( const OpCodeHandler& opInstance );

		friend void LinkClientFunctions( void );
	};
};