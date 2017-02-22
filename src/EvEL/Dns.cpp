#include "DnsClientSocket.h"
#include "Random.h"
#include "EvLoop.h"
#include "Dns.h"
#include "Gev.h"

namespace Evel {

namespace {

bool is_a_space( char c ) {
    return c == ' ' or c == '\t';
}

bool is_a_curl( char c ) {
    return ( c >= '0' and c <= '9' ) or
           ( c >= 'a' and c <= 'z' ) or
           ( c >= 'A' and c <= 'Z' ) or
           c == '.' or c == ':';
}

}

Dns::Dns( EvLoop *ev ) : cur_server( 0 ), ev( ev ? ev : gev.ptr() ) {
    // read entries in /etc/resolv.conf
    std::ifstream f( "/etc/resolv.conf" );
    if ( ! f ) {
        perror( "Pb opening /etc/resolv.conf" );
        abort();
    }

    std::string line;
    while ( std::getline( f, line ) ) {
        const char *d = line.data(), *e = d + line.size();
        while ( d < e and is_a_space( *d ) )
            ++d;
        if ( d == e or *d == '#' or *d == ';' )
            continue;
        if ( e - d > 11 && strncmp( d, "nameserver ", 11 ) == 0 ) {
            const char *b = d += 11;
            while ( d < e && is_a_curl( *d ) )
                ++d;
            server_ips.emplace_back( b, d );
        }
    }
    if ( server_ips.empty() ) {
        std::cerr << "/etc/resolv.conf seems to be empty" << std::endl;
        abort();
    }

    // open a socket
    socket = new DnsClientSocket( this );
    *this->ev << socket;
}

void Dns::query( const std::string &addr, std::function<void (int, const std::vector<InetAddress> &)> cb ) {
    // we already have the entry ?
    std::map<std::string,Entry>::iterator iter_entries = cached_entries.find( addr );
    if ( iter_entries != cached_entries.end() )
        return cb( 0, iter_entries->second.ip_addrs );

    // IMPORTANT_TODO: try several servers, add timeout !!
    std::map<std::string,Wcb>::iterator iter_waiting = waiting_reqs.find( addr );
    if ( iter_waiting != waiting_reqs.end() ) {
        iter_waiting->second.cbs.push_back( cb );
        return;
    }

    // else, make a query
    if ( server_ips.empty() )
        return cb( NO_RESOLV_ENTRY, {} );
    BinStream<DnsConnection> bs( socket->connection( { server_ips[ cur_server ], 53 } ) );

    // make a num_request
    static Random rand;
    unsigned num_request = 0;
    rand.get( &num_request, 2 );

    // header
    bs.write_be16( num_request ); // num_request
    bs.write_byte( ( 1 << 0 ) + // recursivity
                   ( 0 << 1 ) + // truncated msg
                   ( 0 << 2 ) + // authoritative answer
                   ( 0 << 3 ) + // type ( -> std request )
                   ( 0 << 7 ) );  // answer ? )
    bs.write_byte( 0x00 ); // !RA, Z=000, RCODE=NOERROR(0000)
    bs.write_be16( 1 ); // QDCOUNT
    bs.write_be16( 0 ); // ANCOUNT
    bs.write_be16( 0 ); // NSCOUNT
    bs.write_be16( 0 ); // ARCOUNT

    // question
    for( const char *b = addr.data(), *e = b; ; ++e ) { // QNAME
        if ( *e == '.' or *e == '@' or *e == 0 ) {
            if ( e - b ) {
                bs.write_byte( e - b );
                bs.write_some( b, e - b );
            }
            if ( *e == 0 )
                break;
            b = e + 1;
        }
    }
    bs.write_byte( 0 );

    bs.write_be16( 0xFF ); /* QTYPE (any) */
    bs.write_be16( 0x01 ); /* QCLASS (internet) */

    bs.flush();

    //
    Wcb &wcb = waiting_reqs[ addr ];
    wcb.num.push_back( num_request );
    wcb.cbs.push_back( cb );
}

} // namespace Evel
