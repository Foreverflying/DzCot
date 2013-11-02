/**
 *  @file       Inc_DzFastNewDel.h
 *  @brief      override global new delete operator in a single host
 *  @author	    Foreverflying <foreverflying@live.cn>
 *  @date       2011/07/21
 *
 *  Override global new delete operator with faster function
 *  DzMalloc DzFree while using DzCot.
 *
 *  @remark
 *      Just include this file in only one cpp file.
 *      A pointer returned by new must be freed in the same cot host.
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
    void* ret = DzMalloc( count );
    if( !ret ){
        throw bad_alloc();
    }
    return ret;
}

void operator delete( void* p ) throw()
{
    DzFree( p );
}

void* operator new[]( size_t count ) throw( bad_alloc )
{
    void* ret = DzMalloc( count );
    if( !ret ){
        throw bad_alloc();
    }
    return ret;
}

void operator delete[]( void* p ) throw()
{
    DzFree( p );
}

void* operator new( size_t count, const nothrow_t& ) throw()
{
    return DzMalloc( count );
}

void operator delete( void* p, const nothrow_t& ) throw()
{
    DzFree( p );
}

void* operator new[]( size_t count, const nothrow_t& ) throw()
{
    return DzMalloc( count );
}

void operator delete[]( void* p, const nothrow_t& ) throw()
{
    DzFree( p );
}

#endif
