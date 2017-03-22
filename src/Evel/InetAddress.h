#pragma once

#include "Containers/TypeConfig.h"
#include "Containers/BitHash.h"
#include <arpa/inet.h>
#include <iostream>
#include <string>

namespace Evel {

/** IPv6 socket address (with port)
*/
class InetAddress {
public:
    InetAddress( const InetAddress &ia, unsigned port ); // same address, another port
    InetAddress( std::string        ip, unsigned port ); // IPVx
    InetAddress( const PI16        *ip, unsigned port ); // IPV6 only
    InetAddress( const PI8         *ip, unsigned port ); // IPV4 only
    InetAddress();

    void         write_to_stream( std::ostream &os ) const;
    unsigned     port           () const;
    std::string  ip             () const;

    bool         operator<      ( const InetAddress &b ) const;
    bool         operator==     ( const InetAddress &b ) const;
    size_t       hash           () const { return bit_hash( sa ); }

    sockaddr_in6 sa;
};

} // namespace Evel
