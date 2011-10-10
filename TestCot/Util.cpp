/********************************************************************
    created:    2011/09/14 0:31
    file:       Util.cpp
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include <gtest/gtest.h>
#include "Util.h"
//#include "../Dzcot/DzFastNewDel.h"

void TestCot( DzRoutine entry, intptr_t context )
{
    int ret = DzRunHost( CP_LOW, CP_LOW, SS_64K, entry, context );
    EXPECT_EQ( DS_OK, ret );
}