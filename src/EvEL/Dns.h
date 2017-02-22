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
        TIMEOUT = 2,
    };

    Dns( EvLoop *ev = 0 ); ///< ev == 0 => use of gev
    ~Dns();

    void                         query          ( const std::string &addr, std::function<void( int err, const std::vector<InetAddress> &ip_addrs )> cb );

    double                       timeout; ///< time in second before considering that a Dns server is not responding
    std::string                  resolv_conf; ///< name of the resolv.conf file

protected:
    struct Wcb : TimeoutEvent {
        ~Wcb();
        virtual void on_timeout() override;

        std::string                                                                           name;
        std::vector<std::function<void( int err, const std::vector<InetAddress> &ip_addrs )>> cbs;
        PI16                                                                                  num;
        Dns                                                                                  *dns;
    };

    struct Entry {
        std::vector<InetAddress> ip_addrs;
        std::string              name;
        unsigned                 ttl;
    };

    enum { max_req_size = 1024 };
    friend class DnsClientSocket;
    friend class Wcb;

    void                        ans             ( const char *data, size_t size );
    bool                        check_name      ( Entry *&entry, Wcb *&wcb, unsigned num_request, const std::string name );
    std::string                 read_cname      ( BinStream<CmString> bs );
    void                        read_resolv_conf();

    std::map<std::string,Entry> cached_entries;   ///<
    std::map<std::string,Wcb>   waiting_reqs;     ///<
    std::vector<InetAddress>    server_ips;       ///< ip coming from /etc/resolv.conf
    unsigned                    cur_server;       ///< num user server (in server_ips)
    DnsClientSocket            *socket;           ///< socket to communicate with DNS server
    char                       *req;              ///< room to prepare the request
    EvLoop                     *ev;               ///<
    bool                        used_resolv;      ///<
};

} // namespace Evel
