EvEP is the "Evident Event Loop". It is basically a C++ wrapper around epoll/IOCP/kqueue to handle asynchronous events in an loop.

It pursues the same goals than the great [libuv library](https://github.com/libuv/libuv), excepted that it has a C++-ish orientation from the beginning, leading to more straightforward and secure APIs.
