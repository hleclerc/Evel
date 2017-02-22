#pragma once

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
    void query( const std::string &addr, std::function<void( int err, const std::vector<InetAddress> &ip_addrs )> cb );

protected:
    friend struct DnsConnection;

    struct Wcb {
        std::vector<unsigned>                                                                 num;
        std::vector<std::function<void( int err, const std::vector<InetAddress> &ip_addrs )>> cbs;
    };
    struct Entry {
        std::vector<InetAddress> ip_addrs;
        std::string              name;
        unsigned                 ttl;
    };

    std::map<std::string,Entry> cached_entries; //
    std::map<std::string,Wcb>   waiting_reqs;   //
    std::vector<std::string>    server_ips;     // ip coming from /etc/resolv.conf
    unsigned                    cur_server;     // num user server (in server_ips)
    DnsClientSocket            *socket;         // socket to communicate with DNS server
    EvLoop                     *ev;
};

} // namespace Evel
