#include "InetAddress.h"
#include <strings.h>
#include <string.h>
#include <stdio.h>

namespace Evel {

InetAddress::InetAddress( const InetAddress &ia, unsigned port ) {
    bzero( &sa, sizeof sa );
    sa.sin6_family = AF_INET6;
    sa.sin6_port = htons( port );
    memcpy( &sa.sin6_addr, &ia.sa.sin6_addr, 16 );
}

InetAddress::InetAddress( std::string ip, unsigned port ) {
    if ( ip.find( ':' ) == std::string::npos )
        ip = "::ffff:" + ip;

    bzero( &sa, sizeof sa );
    sa.sin6_family = AF_INET6;
    sa.sin6_port = htons( port );
    if ( inet_pton( AF_INET6, ip.c_str(), &sa.sin6_addr ) != 1 )
        perror( "inet_pton" );
}

InetAddress::InetAddress( const PI16 *ip, unsigned port ) {
    bzero( &sa, sizeof sa );
    sa.sin6_family = AF_INET6;
    sa.sin6_port = htons( port );
    memcpy( &sa.sin6_addr, ip, 16 );
}

InetAddress::InetAddress( const PI8 *ip, unsigned port ) {
    bzero( &sa, sizeof sa );
    sa.sin6_family = AF_INET6;
    sa.sin6_port = htons( port );

    PI16 v = 0xFFFF;
    memcpy( (char *)&sa.sin6_addr + 10, &v, 2 );
    memcpy( (char *)&sa.sin6_addr + 12, ip, 4 );
}

InetAddress::InetAddress() {
    bzero( &sa, sizeof sa );
}

void InetAddress::write_to_stream( std::ostream &os ) const {
    os << ip() << ":" << port();
}

unsigned InetAddress::port() const {
    return ntohs( sa.sin6_port );
}

std::string InetAddress::ip() const {
    char ipstr[ INET6_ADDRSTRLEN ];
    inet_ntop( AF_INET6, &sa.sin6_addr, ipstr, sizeof ipstr );
    return ipstr;
}

bool InetAddress::operator<( const InetAddress &b ) const {
    return memcmp( &sa, &b.sa, sizeof sa ) < 0;
}

bool InetAddress::operator==( const InetAddress &b ) const {
    return memcmp( &sa, &b.sa, sizeof sa ) == 0;
}

} // namespace Evel
