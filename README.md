super-fast-webserver
====================

Preface
-------

First off, I need to think of a better name. Secondly, for now this is purely an educational project; I originally planned to implement HTTP/2
support, until I realized HTTP/2 is a binary protocol, which would involve significant modification such that a complete redesign would be necessary. Not worth it for a non-commercial hobby project.

Aim
---

To write a HTTP/1.1 compliant server capable of (at least) serving static content with as high a throughput and low a latency as possible.

Design
------

To make the server as fast as possible I've made the following design decisions:

* Single-threaded event-loop based design. A single epoll instance is epoll()'ed until either a new client connects or a socket of an exising client becomes readable or writable.
* Zero-copy I/O - sendfile() is used to copy file contents directly to socket
* No malloc()'ing or free()'ing during server operation. All data is read into fixed-length buffers which are allocated on startup. If a buffer becomes full the client is disconnected.
* Shedding traffic when number of active clients reaches a specific number.
* Minimal parsing - Be liberal in what we accept. "G / HTTP/1.1" is equiuvalent to "GET / HTTP/1.1"

TODOs
-----

* In client sturct, get rid of the 'stage' field and replace with a function pointer to the handler function for the specific stage. This will cut out some branching. Document the stages in client.h
* Finish request parsing
* Investigate having a worker thread for each CPU
* Catch slow-running requests and close them (will require a thread)
* When max connections is reached temporarially shut down the listener instead of accept()'ing the new connection then immediately close()'ing it.
* Create some testing framework so I can compare to NGINX, Apache, etc.

Future Plans
------------

* Set environment variables
* Cache commonly requested files in memory
* Support for CGI/PHP scripts
* Reverse proxy feature
