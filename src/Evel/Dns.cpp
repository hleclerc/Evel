#include "UdpSocket.h"
#include "Random.h"
#include "EvLoop.h"
#include "Dns.h"
#include "Gev.h"
#include <string.h>
#include <fstream>

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

///
class DnsClientSocket : public UdpSocket {
public:
    DnsClientSocket( Dns *dns ) : UdpSocket( 2048, false ), dns( dns ) {}
    virtual void parse( const InetAddress &src, char **data, unsigned size ) override { dns->ans( *data, size ); }
    Dns *dns;
};

Dns::Dns( EvLoop *ev ) : cur_server( 0 ), req( 0 ), ev( ev ? ev : gev.ptr() ) {
    timeout     = 5.0;
    resolv_conf = "/etc/resolv.conf";
    used_resolv = false;
    socket      = new DnsClientSocket( this );

    *this->ev << socket;
}

Dns::~Dns() {
    if ( req )
        free( req );
}

void Dns::query( const std::string &addr, std::function<void (int, const std::vector<InetAddress> &)> cb ) {
    // we already have the entry ?
    std::map<std::string,Entry>::iterator iter_entries = cached_entries.find( addr );
    if ( iter_entries != cached_entries.end() )
        return cb( 0, iter_entries->second.ip_addrs );

    // we already have a pending request for this addr ?
    std::map<std::string,Wcb>::iterator iter_waiting = waiting_reqs.find( addr );
    if ( iter_waiting != waiting_reqs.end() ) {
        iter_waiting->second.cbs.push_back( cb );
        return;
    }

    // else, make a query
    if ( ! req ) req = (char *)malloc( max_req_size );
    Hpipe::CmQueue cm( req, req + max_req_size );
    Hpipe::BinStream<Hpipe::CmQueue> bs( &cm );
    // socket->connection( { server_ips[ cur_server ], 53 } )

    // make a num_request
    static Random rand;
    PI16 num_request = 0;
    rand.get( &num_request, 2 );

    // reg callback
    Wcb &wcb = waiting_reqs[ addr ];
    ev->inc_nb_waiting_events();
    wcb.cbs.push_back( cb );
    wcb.name = addr;
    wcb.num  = num_request;
    wcb.dns  = this;

    ev->add_timeout( &wcb, timeout );

    // header
    bs.write_be16( num_request ); // num_request
    bs.write_byte( ( 1 << 0 ) +   // recursivity
                   ( 0 << 1 ) +   // truncated msg
                   ( 0 << 2 ) +   // authoritative answer
                   ( 0 << 3 ) +   // type ( -> std request )
                   ( 0 << 7 ) );  // answer ? )
    bs.write_byte( 0x00 );        // !RA, Z=000, RCODE=NOERROR(0000)
    bs.write_be16( 1 );           // QDCOUNT
    bs.write_be16( 0 );           // ANCOUNT
    bs.write_be16( 0 );           // NSCOUNT
    bs.write_be16( 0 );           // ARCOUNT

    // question
    for( const char *b = addr.c_str(), *e = b; ; ++e ) { // QNAME
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

    // send
    if ( server_ips.empty() ) {
        if ( ! used_resolv ) {
            used_resolv = true;
            read_resolv_conf();
        }
        if ( server_ips.empty() )
            return cb( NO_RESOLV_ENTRY, {} );
    }
    // open a socket
    socket->send( server_ips[ 0 ], &req, cm.size(), true );
}

void Dns::ans( const char *data, size_t size ) {
    // header
    Hpipe::CmString cm( data, data + size );
    Hpipe::BinStream<Hpipe::CmString> bs( &cm );
    unsigned num_request  = bs.read_be16();
    unsigned flags_0      = bs.read_byte(); // flags 0
    unsigned flags_1      = bs.read_byte(); // flags 1
    unsigned nb_questions = bs.read_be16(); // QDCOUNT
    unsigned nb_answers   = bs.read_be16(); // ANCOUNT
    unsigned nb_domauth   = bs.read_be16(); // NSCOUNT
    unsigned nb_addsect   = bs.read_be16(); // ARCOUNT
    if ( 0 )
        P( flags_0, flags_1 );

    // questions
    Dns::Entry *entry = 0;
    Dns::Wcb   *wcb   = 0;
    while ( nb_questions-- ) {
        std::string name   = read_cname( bs );
        unsigned    qtype  = bs.read_be16();
        unsigned    qclass = bs.read_be16();
        if ( qtype == 255 && qclass == 1 && ! check_name( entry, wcb, num_request, name ) )
            return;
    }

    // answers
    for( unsigned na = nb_answers + nb_domauth + nb_addsect * 0; na--; ) {
        std::string name;
        unsigned c = bs.read_byte();
        if ( c >= 128 + 64 ) { // pointer format
            c = ( ( c - ( 128 + 64 ) ) << 8 ) + bs.read_byte();
            Hpipe::CmString nc( data + std::min( c, unsigned( size ) ), data + size );
            name = read_cname( &nc );
        } else { // CNAME format
            name = read_cname( bs );
        }

        unsigned type   = bs.read_be16();
        unsigned qclass = bs.read_be16();
        unsigned ttl    = bs.read_be32();
        unsigned rlen   = bs.read_be16();

        if ( type == 0x01 and rlen == 4 ) { // A
            if ( qclass == 1 and not check_name( entry, wcb, num_request, name ) )
                return;
            PI8 ip[ 4 ];
            entry->ttl = ttl;
            bs.read_some( ip, 4 );
            entry->ip_addrs.emplace_back( ip, 0 );
        } else if ( type == 28 and rlen == 16 ) { // AAAA
            if ( qclass == 1 and not check_name( entry, wcb, num_request, name ) )
                return;
            PI16 ip[ 8 ];
            entry->ttl = ttl;
            bs.read_some( ip, 16 );
            entry->ip_addrs.emplace_back( ip, 0 );
        } else
            bs.skip_some( rlen );
    }

    // callbacks
    if ( wcb ) {
        for( std::function<void( int err, const std::vector<InetAddress> &addr )> &f : wcb->cbs )
            f( 0, entry->ip_addrs );
        waiting_reqs.erase( entry->name );
    }
}

std::string Dns::read_cname( Hpipe::BinStream<Hpipe::CmString> bs ) {
    std::string name;
    while ( true ) {
        unsigned c = bs.read_byte();
        if ( not bs.buf->ack_read_some( c ) ) {
            ev->err( "Pb with DNS lookup (reading name in answers)" );
            return "";
        }
        if ( not c )
            break;
        if ( name.size() )
            name += '.';
        name += std::string( bs.ptr(), bs.ptr() + c );
        bs.skip_some( c );
    }
    return name;
}

void Dns::read_resolv_conf() {
    // read entries in /etc/resolv.conf
    std::ifstream f( "/etc/resolv.conf" );
    if ( ! f ) {
        ev->err( "Pb opening /etc/resolv.conf", strerror( errno ) );
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
            server_ips.emplace_back( std::string( b, d ), 53 );
        }
    }

    if ( server_ips.empty() )
        ev->err( resolv_conf + " seems to be empty" );
}

bool Dns::check_name( Entry *&entry, Wcb *&wcb, unsigned num_request, const std::string name ) {
    //
    if ( entry )
        return entry->name == name;

    //
    std::map<std::string,Dns::Wcb>::iterator iter = waiting_reqs.find( name );
    if ( iter == waiting_reqs.end() ) {
        ev->log( "Unrequested dns info (can be a late answer for instance)" );
        return false;
    }
    wcb = &iter->second;
    if ( wcb->num != num_request ) {
        ev->log( "Dns num_request answer is not correct (it is an attack ?)" );
        return false;
    }

    entry = &cached_entries[ name ];
    entry->name = name;
    return true;
}

Dns::Wcb::~Wcb() {
    dns->ev->dec_nb_waiting_events();
}

void Dns::Wcb::on_timeout() {
    for( std::function<void( int err, const std::vector<InetAddress> &addr )> &f : cbs )
        f( TIMEOUT, {} );
    dns->waiting_reqs.erase( name );
}

} // namespace Evel
