#include "../src/Evel/ExpIndexedList.h"
#include <gtest/gtest.h>
#include <map>
using namespace Evel;

struct Foo {
    void write_to_stream( std::ostream &os ) const {
        os << exp_indexed_item_data.offset;
    }

    ExpIndexedItemData<Foo,GetExpIndexedItemData> exp_indexed_item_data;
};

TEST( ExpIndexedList, basic_operations ) {
    ExpIndexedList<Foo,GetExpIndexedItemData,4,3> el;
    // of=0 of=3 of=9 of=21 of=45 of=93
    std::map<Foo **,int> nm;
    constexpr int n = 150;
    Foo f[ n ] = {};

    for( unsigned i = 0; i < n; ++i )
        el.add( f + i, i );

    // check that items are on the right bucket
    int sol[] = {
         1,  2,  3,  4,  5,  5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9,  9,
         9,  9,  9,  9,  9,  9,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11,
        11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13,
        13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    };
    for( unsigned i = 0; i < n; ++i ) {
        auto ptr = f[ i ].exp_indexed_item_data.lst;
        if ( ! nm.count( ptr ) )
            nm[ ptr ] = nm.size();
        EXPECT_EQ( nm[ ptr ], i < 100 ? sol[ i ] : 13 );
    }

    //
    for( unsigned i = 0; i < n; ++i ) {
       // P( el );
       std::vector<int> indices;
       el.shift( [&]( Foo *foo ) {
           int b = foo->exp_indexed_item_data.offset;
           indices.push_back( b );
       } );
       EXPECT_EQ( indices.size(), 1 );
       EXPECT_EQ( indices[ 0 ], i );
   }
}

TEST( ExpIndexedList, rem ) {
    ExpIndexedList<Foo,GetExpIndexedItemData,4,3> el;
    constexpr int n = 150;
    Foo f[ n ] = {};

    for( unsigned i = 0; i < n; ++i )
        el.add( f + i, i );


    // rem some elements
    for( unsigned i = 1; i < n; i += 2 )
        el.rem( f + i );

    //
    for( unsigned i = 1; i < n; i += 2 ) {
       // P( el );
       std::vector<int> a, b;
       el.shift( [&]( Foo *foo ) {
           a.push_back( foo->exp_indexed_item_data.offset );
       } );
       el.shift( [&]( Foo *foo ) {
           b.push_back( foo->exp_indexed_item_data.offset );
       } );
       EXPECT_EQ( b.size(), 0 );
       EXPECT_EQ( a.size(), 1 );
       EXPECT_EQ( a[ 0 ], i - 1 );
   }
}

TEST( ExpIndexedList, destroy ) {
    ExpIndexedList<Foo,GetExpIndexedItemData,4,3> el;
    {
        constexpr int n = 10;
        Foo f[ n ] = {};
        for( unsigned i = 0; i < n; ++i )
            el.add( f + i, i );

        EXPECT_EQ( el.size(), 10 );
    }

    EXPECT_EQ( el.size(), 0 );
}
