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

/*
	ICECAT Networking		v2.0

	file IceNetTypedefs.h:

	This file contains a number for commonly used typedefs and prototypes.
*/

namespace IceNet
{
	struct ConnectionInfo;

	class Packet;
	class ClientProxy;
	class Client;

	typedef void ( *PACKET_HANDLING_FUNCTION )( Packet* );
	typedef void ( *VOID_WITH_CONNECTIONINFO_PARAM )( ConnectionInfo );
	typedef void ( *VOID_WITH_CLIENTPROXY_PARAM )( ClientProxy* );
	typedef void ( *VOID_WITH_CLIENT_PARAM )( Client* );
	typedef void ( *VOID_WITH_NO_PARAM )( void );
}
