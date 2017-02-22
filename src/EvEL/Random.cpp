#include <openssl/rand.h>
#include "Random.h"

namespace Evel {

Random::Random() {
    if ( RAND_load_file( "/dev/urandom", 128 ) != 128 ) {
        std::cerr << "Pb seeding..." << std::endl;
        abort();
    }
}

bool Random::get( void *buf, int len ) {
    return RAND_bytes( (unsigned char *)buf, len );
}

} // namespace Evel
