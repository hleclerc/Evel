#pragma once

#include "SslConnection.h"
#include <functional>

namespace Evel {

/**
*/
class SslConnection_WF : public SslConnection {
public:
    using TF_rdy           = std::function<void( SslConnection_WF *c )>;
    using TF_parse         = std::function<void( SslConnection_WF *c, char **data, size_t size, size_t rese )>;
    using TF_close         = std::function<void( SslConnection_WF *c )>;

    using SslConnection::SslConnection;
    ~SslConnection_WF();

    TF_rdy           f_rdy;
    TF_parse         f_parse;
    TF_close         f_close;

protected:
    virtual void     on_rdy() override;
    virtual void     parse ( char **data, size_t size, size_t rese ) override;
};


} // namespace Evel
