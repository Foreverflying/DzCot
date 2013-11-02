/**
 *  @file       Inc_DzFastNewDelEx.h
 *  @brief      override global new delete operator in multi cot hosts
 *  @author	    Foreverflying <foreverflying@live.cn>
 *  @date       2011/10/23
 *
 *  Override global new delete operator with faster function
 *  DzMallocEx DzFreeEx while using DzCot.
 *
 *  @remark
 *      Just include this file in only one cpp file.
 *      A pointer returned by new can be freed in other cot hosts.
 */

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