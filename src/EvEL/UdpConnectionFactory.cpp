#include "UdpConnectionFactory.h"
#include "UdpConnection.h"

namespace Evel {

UdpConnectionFactory::UdpConnectionFactory() : nb_live_conn( 0 ), want_close_fact( false ) {
}

UdpConnectionFactory::~UdpConnectionFactory() {
}

UdpConnection *UdpConnectionFactory::connection( const InetAddress &dst, bool create_if_new ) {
    TMap::iterator iter = connections.find( dst );
    if ( iter == connections.end() )
        return create_if_new ? connections.insert( iter, std::make_pair( dst, factory( dst ) ) )->second.get() : 0;
    return iter->second.get();
}

UdpConnectionFactory::TMap::iterator UdpConnectionFactory::map_iter( const InetAddress &dst, bool create_if_new ) {
    TMap::iterator iter = connections.find( dst );
    if ( iter == connections.end() && create_if_new )
        return connections.insert( iter, std::make_pair( dst, factory( dst ) ) );
    return iter;
}

void UdpConnectionFactory::send( const InetAddress &dst, const char **data, size_t size, bool allow_transfer_ownership ) {
    connection( dst )->send( data, size, allow_transfer_ownership );
}

void UdpConnectionFactory::dec_nb_live_conn() {
    if ( --nb_live_conn == 0 && want_close_fact )
        Event::close();
}

void UdpConnectionFactory::parse( const InetAddress &src, char **data, unsigned size ) {
    if ( next_del )
        return;
    connection( src )->_parse( data, size );
}

void UdpConnectionFactory::close() {
    if ( want_close_fact )
        return;
    want_close_fact = true;
    if ( nb_live_conn == 0 )
        Event::close();
}

void UdpConnectionFactory::del( const InetAddress &src ) {
    TMap::iterator iter = map_iter( src );
    if ( iter != connections.end() )
        connections.erase( iter );
}

void UdpConnectionFactory::work() {
    while ( UdpConnection *conn = to_del.pop_front() )
        del( conn->addr );
}

bool UdpConnectionFactory::out_are_sent() const {
    return UdpSocket::out_are_sent() && nb_live_conn == 0;
}


} // namespace Evel
