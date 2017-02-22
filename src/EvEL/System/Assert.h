#pragma once

namespace Evel {

void abort_or_throw();
bool __disp_and_abort_if_not_cond__( bool cond, const char *txt, ... );
bool __disp_if_not_cond__( bool cond, const char *txt, ... );
void __disp( const char *txt, ... );
bool __do_nothing__();

#ifdef DEBUG
    #define ASSERT_IF_DEBUG( A ) Evel::__disp_and_abort_if_not_cond__( A, "%s:%i: assertion %s not checked\n", __FILE__, __LINE__, #A )
#else
    #define ASSERT_IF_DEBUG( A ) // Evel::__do_nothing__()
#endif // DEBUG

#define ASSERT( A, txt, ... ) Evel::__disp_and_abort_if_not_cond__( A, "%s:%i: assertion %s not checked -> " txt "\n", __FILE__, __LINE__, #A, ##__VA_ARGS__ )
#define WASSERT( A, txt, ... ) Evel::__disp_if_not_cond__( A, "%s:%i: assertion %s not checked -> " txt "\n", __FILE__, __LINE__, #A, ##__VA_ARGS__ )
#define ERROR( txt, ... ) Evel::__disp_and_abort_if_not_cond__( 0, "%s:%i: " txt "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#define WARNING( A, ... ) Evel::__disp( "%s:%i: " #A "\n", __FILE__, __LINE__, ##__VA_ARGS__ )

#define TODO ASSERT( 0, "TODO" )
#define IMPORTANT_TODO ASSERT( 0, "IMPORTANT TODO" )

}
