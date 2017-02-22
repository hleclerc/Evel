#include "../src/Evel/CiList.h"
#include <gtest/gtest.h>
using namespace Evel;

struct Item {
    Item( int val ): val( val ) {}
    Item *next = 0;
    int   val;
};

TEST( CiList, basic_operations ) {
    Item items[] = { 0, 1, 2, 3, 4 };
    CiList<Item> lst;

    lst.push_back( items + 0 );
    lst.push_back( items + 1 );
    lst.push_back( items + 2 );

    EXPECT_EQ( lst.empty(), false );
    EXPECT_EQ( lst.pop_front()->val, 0 );
    EXPECT_EQ( lst.pop_front()->val, 1 );
    EXPECT_EQ( lst.pop_front()->val, 2 );
    EXPECT_EQ( lst.pop_front(), nullptr );
    EXPECT_EQ( lst.empty(), true );
}

TEST( CiList, prepend_11 ) {
    Item items[] = { 0, 1, 2, 3, 4 };
    CiList<Item> a, b;

    a.push_back( items + 0 );
    a.push_back( items + 1 );
    b.push_back( items + 2 );
    b.push_back( items + 3 );

    b.prepend( a );
    for( int i = 0; i < 4; ++i )
        EXPECT_EQ( b.pop_front()->val, i );
    EXPECT_EQ( b.empty(), true );
}

TEST( CiList, prepend_10 ) {
    Item items[] = { 0, 1, 2, 3, 4 };
    CiList<Item> a, b;

    a.push_back( items + 0 );
    a.push_back( items + 1 );
    a.push_back( items + 2 );
    a.push_back( items + 3 );

    b.prepend( a );
    for( int i = 0; i < 4; ++i )
        EXPECT_EQ( b.pop_front()->val, i );
    EXPECT_EQ( b.empty(), true );
}

TEST( CiList, prepend_01 ) {
    Item items[] = { 0, 1, 2, 3, 4 };
    CiList<Item> a, b;

    b.push_back( items + 0 );
    b.push_back( items + 1 );
    b.push_back( items + 2 );
    b.push_back( items + 3 );

    b.prepend( a );
    for( int i = 0; i < 4; ++i )
        EXPECT_EQ( b.pop_front()->val, i );
    EXPECT_EQ( b.empty(), true );
}

TEST( CiList, prepend_00 ) {
    CiList<Item> a, b;
    b.prepend( a );
    EXPECT_EQ( b.empty(), true );
}

