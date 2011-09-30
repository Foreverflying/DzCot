/********************************************************************
    created:    2011/09/14 0:31
    file:       Util.cpp
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "Util.h"
//#include "../Dzcot/DzFastNewDel.h"

void TestCot( DzRoutine entry, void* context )
{
    DzRunHost( CP_LOW, CP_LOW, SS_64K, entry, context );
}