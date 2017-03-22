#pragma once

#include <Hpipe/CbQueue.h>
#include "Print.h"

namespace Evel {

/**
  T get a class like TlsConnection, TcpConnection, ... with a parse( buffer, last_buf, data = 0, end_m1 )

  enable notably direct use of code generated by hpipe
*/
template<class T>
class HpipeBufferParseAdapter : public T {
public:
    using T::T;

    using T::send;
    void             send( Hpipe::CbQueue &&cq );

protected:
    virtual void     on_parse_error();
    virtual unsigned parse         ( Hpipe::Buffer *buf, bool last_buf, const unsigned char *data = 0, const unsigned char *end_m1 = 0 ) = 0; ///< will be redefined by hpipe

private:
    virtual void     parse         ( char **data, size_t size, size_t rese ) override;
    virtual size_t   offset_parse  () const override; ///< reserve room before data to parse: we want to allocate Hpipe::Buffer(s)
};

template<class T>
void HpipeBufferParseAdapter<T>::send( Hpipe::CbQueue &&cq ) {
    cq.data_visitor( [&]( const unsigned char *beg, const unsigned char *end ) {
        send( (const char *)beg, end - beg );
    } );
}

template<class T>
size_t HpipeBufferParseAdapter<T>::offset_parse() const {
    return sizeof( Hpipe::Buffer ) - Hpipe::Buffer::nb_in_base_data;
}

template<class T>
void HpipeBufferParseAdapter<T>::parse(char **data, size_t size, size_t rese) {
    Hpipe::Buffer *buf = new ( *data ) Hpipe::Buffer( size, rese );
    if ( parse( buf, false ) == 2 )
        on_parse_error();
    if ( buf->cpt_use )
        *data = 0;
}

template<class T>
void HpipeBufferParseAdapter<T>::on_parse_error() {
    this->close();
}

} // namespace Evel
