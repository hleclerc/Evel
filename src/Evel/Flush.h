#pragma once

namespace Evel {

struct Flush {
    template<class Bq>
    void write_to( Bq bq ) const {
        bq.buf->flush();
    }
};

constexpr Flush flush;

} // namespace Evel
