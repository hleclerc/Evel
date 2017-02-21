#pragma once

#include "TcpConnection.h"
#include <functional>

namespace Evel {

/**
*/
class TcpConnection_WF : public TcpConnection {
public:
    using TF_rdy           = std::function<void( TcpConnection_WF *c )>;
    using TF_parse         = std::function<void( TcpConnection_WF *c, char **data, size_t size, size_t rese )>;
    using TF_close         = std::function<void( TcpConnection_WF *c )>;

    using TcpConnection::TcpConnection;
    ~TcpConnection_WF();

    TF_rdy           f_rdy;
    TF_parse         f_parse;
    TF_close         f_close;

protected:
    virtual void     on_rdy             () override;
    virtual void     parse              ( char **data, size_t size, size_t rese ) override;
};


} // namespace Evel
