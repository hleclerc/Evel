#include "DnsClientSocket.h"

namespace Evel {

namespace {

std::string read_cname( BinStream<CmString> bs ) {
    std::string name;
    while ( true ) {
        unsigned c = bs.read_byte();
        if ( not bs.buf->ack_read_some( c ) ) {
            DISP_ERROR( "Pb with DNS lookup (reading name in answers)" );
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

} // namespace

DnsClientSocket::DnsClientSocket( Dns *dns ) {
}

void DnsClientSocket::parse( const InetAddress &src, char **data, unsigned size ) {

}


//bool check_name( Dns::Entry *&entry, Dns::Wcb *&wcb, unsigned num_request, const std::string name ) {
//    //
//    if ( entry )
//        return entry->name == name;

//    //
//    std::map<std::string,Dns::Wcb>::iterator iter = dns->waiting_reqs.find( name );
//    if ( iter == dns->waiting_reqs.end() ) {
//        DISP_INFO( "Unrequested dns info (can be a late answer for instance)" );
//        return false;
//    }
//    wcb = &iter->second;
//    if ( not wcb->num.contains( num_request ) ) {
//        DISP_INFO( "Dns num_request answer is not correct (it is an attack ?)" );
//        return false;
//    }

//    entry = &dns->cached_entries[ name ];
//    entry->name = name;
//    return true;
//}

//void parse( char *data, PT size ) {
//    // header
//    CmString cm( data, data + size );
//    BinStream<CmString> bs( &cm );
//    unsigned num_request  = bs.read_be16();
//    unsigned flags_0      = bs.read_byte(); // flags 0
//    unsigned flags_1      = bs.read_byte(); // flags 1
//    unsigned nb_questions = bs.read_be16(); // QDCOUNT
//    unsigned nb_answers   = bs.read_be16(); // ANCOUNT
//    unsigned nb_domauth   = bs.read_be16(); // NSCOUNT
//    unsigned nb_addsect   = bs.read_be16(); // ARCOUNT
//    if ( 0 )
//        PRINT( flags_0, flags_1 );

//    // questions
//    Dns::Entry *entry = 0;
//    Dns::Wcb   *wcb   = 0;
//    while ( nb_questions-- ) {
//        std::string name   = read_cname( bs );
//        unsigned    qtype  = bs.read_be16();
//        unsigned    qclass = bs.read_be16();
//        if ( qtype == 255 and qclass == 1 and not check_name( entry, wcb, num_request, name ) )
//            return;
//    }

//    // answers
//    for( unsigned na = nb_answers + nb_domauth + nb_addsect * 0; na--; ) {
//        std::string name;
//        unsigned c = bs.read_byte();
//        if ( c >= 128 + 64 ) { // pointer format
//            c = ( ( c - ( 128 + 64 ) ) << 8 ) + bs.read_byte();
//            CmString nc( data + std::min( c, unsigned( size ) ), data + size );
//            name = read_cname( &nc );
//        } else { // CNAME format
//            name = read_cname( bs );
//        }

//        unsigned type   = bs.read_be16();
//        unsigned qclass = bs.read_be16();
//        unsigned ttl    = bs.read_be32();
//        unsigned rlen   = bs.read_be16();

//        if ( type == 0x01 and rlen == 4 ) { // A
//            if ( qclass == 1 and not check_name( entry, wcb, num_request, name ) )
//                return;
//            entry->ttl = ttl;
//            bs.read_some( entry->ipv4.push_back(), 4 );
//        } else if ( type == 28 and rlen == 16 ) { // AAAA
//            if ( qclass == 1 and not check_name( entry, wcb, num_request, name ) )
//                return;
//            entry->ttl = ttl;
//            bs.read_some( entry->ipv6.push_back(), 16 );
//        } else
//            bs.skip_some( rlen );
//    }

//    // callbacks
//    if ( wcb ) {
//        for( std::function<void( const Vec<Dns::Ipv4> &ipv4, const Vec<Dns::Ipv6> &ipv6 )> &f : wcb->ok )
//            f( entry->ipv4, entry->ipv6 );
//        dns->waiting_reqs.erase( entry->name );
//    }
//}


} // namespace Evel
