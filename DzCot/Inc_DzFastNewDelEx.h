/********************************************************************
    created:    2011/10/23 21:09
    file:       Inc_DzFastNewDelEx.h
    author:     Foreverflying
    purpose:    Override the global new delete operator with faster
                function DzMallocExEx DzFreeExEx while using DzCot and
                need delete objects which new by another CotHost.
                just include this file in only one cpp file.
*********************************************************************/

#ifdef __cplusplus

#include <new>
#include <exception>
#include "Inc_DzCot.h"

using namespace std;

#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif

void* operator new( size_t count ) throw( bad_alloc )
{
    void* ret = DzMallocEx( count );
    if( !ret ){
        throw bad_alloc();
    }
    return ret;
}

void operator delete( void* p ) throw()
{
    DzFreeEx( p );
}

void* operator new[]( size_t count ) throw( bad_alloc )
{
    void* ret = DzMallocEx( count );
    if( !ret ){
        throw bad_alloc();
    }
    return ret;
}

void operator delete[]( void* p ) throw()
{
    DzFreeEx( p );
}

void* operator new( size_t count, const nothrow_t& ) throw()
{
    return DzMallocEx( count );
}

void operator delete( void* p, const nothrow_t& ) throw()
{
    DzFreeEx( p );
}

void* operator new[]( size_t count, const nothrow_t& ) throw()
{
    return DzMallocEx( count );
}

void operator delete[]( void* p, const nothrow_t& ) throw()
{
    DzFreeEx( p );
}

#endif