IceNet
======

IceNet is a statically-linked low-level networking library for Windows. It is currently in early development stages.

Features
--------

IceNet provides both server and client contexts. The library is low-level, meaning that, beyond basic operations, users are expected to handle data in packets manually through an opcode system. IceNet is designed to be suitable for single-purpose servers; all clients exists within the same context, meaning this library will not be suitable for server applications which provide a large amount of services. IceNet is more suitable for server applications such as those for video games, voice-over-ip and chat.

Feature sheet:

- TCP and UDP support.
- Flexible; packets can convey any type of information and are handled by user code.
- Fully multithreaded and scalable. Each client connected to the server is given it's own threads which send, receive or handle packets. IceNet is asynchronous by nature.*
- Compact. Initializing the server/client takes only a few lines of code.

Caveats:

- UDP packets are fixed to 256b of size.
- Encryption of packets in whole is not possible as of yet.
- Users are expected to anticipate connection success or failure.
- No direct P2P.

*Packet handling can be done synchronously.

History
-------

The first version of IceNet was created in September 2012. It was used in a multiplayer first-person-shooter game. This version was very buggy.

The second version of IceNet was created from December 2012 to January 2013. It was to be used as part of a networking assignment. A crippling bug caused suspension of development on IceNet.

Development was resumed in January 2014. The crippling bug was fixed, which allowed for the first public release of IceNet.

Acknowledgements
----------------

Juul Joosten, for commenting on the code (back in 2012/2013).

How-to
======

In IceNet, each client has it's own public and private ID. The public ID is used to identify the client everywhere (on other clients) while the private ID is used by the server to find out the origin and handle packet destinations.

The usage of IceNet in an application is very easy. The project should be linked with the appropiate .lib file (IceNetLib.lib for release and IceNetLib_d.lib for debug). Then, the IceNet headers should be included in the project.

For a server application:
```cpp
#include "IceNetServer.h"
using namespace IceNet::ServerSide;
```
For a client application:
```cpp
#include "IceNetClient.h",
using namespace IceNet::ClientSide;
```

Before initializing/connecting, you should link opcode numbers to functions. Some functions are linked internally. Example;

```cpp
// PRINT_NUMBER_OPCODE is an enum
LinkOpCodeFunction( PRINT_NUMBER_OPCODE, PrintNumber );
```
Some functions are called internally, here is an example;

```cpp
// This function is called on all clients when another client connects to the server.
ClientSide::SetOnAddRemoteClient( AddRemoteClient );

// These functions are called depending on the result of the connection.
ClientSide::SetOnConnectionSucceed( ConnectionSuccessful );
ClientSide::SetOnConnectionFail( ConnectionFailed );

// These functions are called when a client joins or leaves.
ServerSide::SetOnAddClient( OnJoin );
ServerSide::SetOnRemoveClient( OnPart );
```

After the functions have been linked, one may initialize the server or connect to it;

```cpp
// Initialize on port 346366. You must set flags as well.
ServerSide::Initialize( "346366", NetworkControl::PROTOCOL_UDP | NetworkControl::HANDLER_ASYNC );

// Connect to IP on port 34366 (localhost).
ClientSide::Connect( "346366", "127.0.0.1", NetworkControl::PROTOCOL_UDP | NetworkControl::HANDLER_SYNC )
```

Here's an example of a packet handling function;

```cpp
void PrintNumber( Packet* packet )
{
  // Retrieve the data from the packet and increment the streaming pointer.
  int number = packet->RetrieveDataStreaming< int >();
  
  printf( "%d\n", number );
  
  // Packet is automatically deleted!
}
```

When using NetworkControl::HANDLER_ASYNC in the initialization, packets are handled by the default client threads, meaning thread safety has to be ensured. If you wish to handle packets in a thread of choice, you may do so;

```cpp
// Use the NetworkControl::HANDLER_SYNC flag
ServerSide::Initialize( "346366", NetworkControl::PROTOCOL_UDP | NetworkControl::HANDLER_SYNC );

// for clients
ClientSide::Connect( "346366", "127.0.0.1", NetworkControl::PROTOCOL_UDP | NetworkControl::HANDLER_SYNC )

....

// Then, in a thread of choice. (This function will return 0 if HANDLER_SYNC isn't used.)
unsigned int packetsHandled = HandlePackets();

// Packets are deleted automatically!
```

To send packets from the client (example);

```cpp
Packet* newPacket = new Packet():

// Setting the public ID is optional. This is useful if the packet is bounced back to other clients. 
newPacket->SetClientPublicId( ClientSide::GetLocalClient()->m_PublicId );

// Setting the private ID is useful if a packet is sent back with info from this packet.
newPacket->SetClientPrivateId( ClientSide::GetLocalClient()->m_PrivateId );

// AddDataStreaming can be used to add any type of data to the packet in a streaming manner.
newPacket->AddDataStreaming< int >( 42 );

// Opcode determines which linked function to call.
newPacket->SetOpcode( 5 );

// Send a package with TCP OR UDP.
ClientSide::SendTCP( newPacket );
ClientSide::SendUDP( newPacket );

// Do not delete newPacket!
```

To send packets from the server (example);

```cpp
Packet* newPacket = new Packet():

// Setting the private ID is required if the packet needs to go to a specific destination.
newPacket->SetClientPrivateId( someClient->m_PrivateId );

....

// Send a package with TCP OR UDP to a specific client...
ServerSide::SendTCP( newPacket );
ServerSide::SendUDP( newPacket );

// Broadcasting options
newPacket->SetUDPEnabled( true ); // Enable UDP broadcast.
newPacket->SetFlag( Packet::PF_FREE ); // Default, send to all.
newPacket->SetFlag( Packet::PF_SPECIFIC ); // Send only to client with the private Id specified by the packet.
newPacket->SetFlag( Packet::PF_EXCLUDEORIGIN ); // Send to all but the client with the private Id specified by the packet.

// ... or broadcast the packet.
ServerSide::Broadcast( newPacket );

// Do not delete newPacket!
```

More examples can be found within the examples folder.

Future
======
- Add Linux support.
- Improve connection handling.
- Add custom protocol support.
- Add encryption.
