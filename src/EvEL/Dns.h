#pragma once

#include "Stream/BinStream.h"
#include "InetAddress.h"
#include "EvLoop.h"
#include <functional>
#include <vector>
#include <string>
#include <map>

namespace Evel {

class DnsClientSocket;

/**
*/
class Dns {
public:
    /// error codes
    enum {
        NO_RESOLV_ENTRY = 1,
    };

    Dns( EvLoop *ev = 0 ); ///< ev == 0 => use of gev
    ~Dns();

    void query( const std::string &addr, std::function<void( int err, const std::vector<InetAddress> &ip_addrs )> cb );

protected:
    friend class DnsClientSocket;
    enum { max_req_size = 1024, };

    struct Wcb {
        std::vector<std::function<void( int err, const std::vector<InetAddress> &ip_addrs )>> cbs;
        PI16                                                                                  num;
    };
    struct Entry {
        std::vector<InetAddress> ip_addrs;
        std::string              name;
        unsigned                 ttl;
    };

    void                        ans       ( const char *data, size_t size );
    bool                        check_name( Entry *&entry, Wcb *&wcb, unsigned num_request, const std::string name );
    std::string                 read_cname( BinStream<CmString> bs );

    std::map<std::string,Entry> cached_entries; ///<
    std::map<std::string,Wcb>   waiting_reqs;   ///<
    std::vector<InetAddress>    server_ips;     ///< ip coming from /etc/resolv.conf
    unsigned                    cur_server;     ///< num user server (in server_ips)
    DnsClientSocket            *socket;         ///< socket to communicate with DNS server
    char                       *req;            ///< room to prepare the request
    EvLoop                     *ev;
};

} // namespace Evel
