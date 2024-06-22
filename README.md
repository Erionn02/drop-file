# drop-file

## Requirements:
1) Server should expose only one port. -> Done
2) Server should be a proxy only, clients should not talk to each other.
3) Server should not store any intermediate files and use RAM only.
4) Whole communication should be encrypted. -> Done
5) Communication is 3 phase:

Phase 0:

Server accepts a connection and distinguishes between 3 options:
- somebody wants to send file
- somebody wants to receive a file
- somebody is trying hijack the communication, abort this connection

Phase 1:

Server registers the sender, collects file metadata and assigns him the special token that receiver will 
use to ask for given resource

Phase 2:

Server waits for receiver for a given token, sends start message to sender client and proxies the data exchange
In this phase the only data that sender sends is file content chunk by chunk, after every chunk sender waits for
some receive confirmation message to pace sender's transfer speed with receiver's one. 
At every point of transfer, server controls total received bytes with the size declared in the beginning