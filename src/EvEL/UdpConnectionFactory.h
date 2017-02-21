#pragma once

#include "UdpConnection.h"
#include <unordered_map>
#include "UdpSocket.h"
#include "CiList.h"
#include <atomic>

namespace Evel {
class UdpConnection;

///
class UdpConnectionFactory : public UdpSocket {
public:
    struct InetAddressHash { std::size_t operator()( const InetAddress &k ) const { return k.hash(); } };
    using TMap = std::unordered_map<InetAddress,std::unique_ptr<UdpConnection>,InetAddressHash>;

    UdpConnectionFactory();
    ~UdpConnectionFactory();

    UdpConnection           *connection     ( const InetAddress &dst, bool create_if_new = true ); ///<
    TMap::iterator           map_iter       ( const InetAddress &dst, bool create_if_new = true ); ///<

    using UdpSocket::send;
    virtual void             send           ( const InetAddress &dst, const char **data, size_t size, bool allow_transfer_ownership = true ) override; ///< if transfer_ownership is allowed, send may take ownership of *data, in which case, *data is changed to null and will be freed using free()

    virtual void             close          () override; ///< delete this after all the outputs has been seen

    void                     inc_nb_live_conn() { ++nb_live_conn; }
    void                     dec_nb_live_conn();

protected:
    friend class UdpConnection;
    struct ND { UdpConnection *&operator()  ( UdpConnection *item ) const { return item->next_del; } };

    virtual UdpConnection   *factory        ( const InetAddress &src ) = 0;
    virtual void             parse          ( const InetAddress &src, char **data, unsigned size );
    virtual void             del            ( const InetAddress &src );
    virtual void             work           () override;
    virtual bool             out_are_sent   () const override;

    TMap                     connections;
    std::atomic<int>         nb_live_conn;
    CiList<UdpConnection,ND> to_del;
    bool                     want_close_fact;
};

} // namespace Evel
