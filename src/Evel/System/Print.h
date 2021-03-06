#pragma once

#include "../Containers/EnableIf.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

namespace Evel {

template<class T>
std::ostream &operator<<( std::ostream &os, const std::vector<T> &val ) {
   if ( ! val.empty() ) {
       os << val[ 0 ];
       for( unsigned i = 1; i < val.size(); ++i )
           os << " " << val[ i ];
   }
   return os;
}

/// classes with `write_to_stream` will be displayed by default with the corresponding methods
template<class T>
typename EnableIf<1,std::ostream,decltype(&T::write_to_stream)>::T &operator<<( std::ostream &os, const T &val ) {
   val.write_to_stream( os );
   return os;
}

template<class OS> void __my_print( OS &os ) { os << std::endl; }
template<class OS,class T0> void __my_print( OS &os, const T0 &t0 ) { os << t0 << std::endl; }
template<class OS,class T0,class... Args> void __my_print( OS &os, const T0 &t0, const Args &...args ) { os << t0 << ", "; __my_print( os, args... ); }

#ifndef P
   #define P( ... ) \
       Evel::__my_print( std::cout << #__VA_ARGS__ " -> ", __VA_ARGS__ );
   #define I( info, ... ) \
       Evel::__my_print( std::cout << info << " ", ##__VA_ARGS__ );
   #define PE( ... ) \
       Evel::__my_print( std::cerr << #__VA_ARGS__ " -> ", __VA_ARGS__ );
   #define PN( ... ) \
       Evel::__my_print( std::cout << #__VA_ARGS__ " ->\n", __VA_ARGS__ );
   #define PL( ... ) \
       Evel::__my_print( std::cout << __FILE__ << ":" << __LINE__ << ": " << #__VA_ARGS__ " -> ", __VA_ARGS__ );
   #define PLE( ... ) \
       Evel::__my_print( std::cerr << __FILE__ << ":" << __LINE__ << ": " << #__VA_ARGS__ " -> ", __VA_ARGS__ );
   #define PF( ... ) \
       Evel::__my_print( std::cout << __PRETTY_FUNCTION__ << ": " << #__VA_ARGS__ " -> ", __VA_ARGS__ );
#endif

template<class T>
std::string to_string( const T &val ) {
   std::ostringstream ss;
   ss << val;
   return ss.str();
}

template<class T>
std::string to_string_hex( const T &val ) {
   std::ostringstream ss;
   ss << std::hex << val;
   return ss.str();
}

} // namespace Evel

