/**
 *  @file       DzCrossOs.h
 *  @brief      
 *  @author	    Foreverflying <foreverflying@live.cn>
 *  @date       2016/02/17
 *
 */

#ifndef __DzCrossOs_h__
#define __DzCrossOs_h__

#if defined( _WIN32 )
#include <WinBase.h>
#ifndef __cplusplus
#define inline __inline
#endif
#endif

#ifdef __cplusplus
extern "C"{
#endif

#if defined( _WIN32 )

static inline
int DzAtomGetInt( int volatile* val )
{
    return *val;
}

static inline
void DzAtomSetInt( int volatile* val, int set )
{
    *val = set;
}

static inline
void* DzAtomGetPtr( void* volatile* val )
{
    return *val;
}

static inline
void DzAtomSetPtr( void* volatile* val, void* set )
{
    *val = set;
}

static inline
int DzAtomIncInt( int volatile* val )
{
    return InterlockedIncrement( (LONG volatile*)val ) - 1;
}

static inline
int DzAtomDecInt( int volatile* val )
{
    return InterlockedDecrement( (LONG volatile*)val ) + 1;
}

static inline
int DzAtomAddInt( int volatile* val, int add )
{
    return InterlockedExchangeAdd( (LONG volatile*)val, (LONG)add );
}

static inline
int DzAtomSubInt( int volatile* val, int sub )
{
    return InterlockedExchangeAdd( (LONG volatile*)val, (LONG)( - sub ) );
}

static inline
int DzAtomOrInt( int volatile* val, int mask )
{
    return _InterlockedOr( (LONG volatile*)val, (LONG)mask );
}

static inline
int DzAtomAndInt( int volatile* val, int mask )
{
    return _InterlockedAnd( (LONG volatile*)val, (LONG)mask );
}

static inline
int DzAtomCasInt( int volatile* val, int cmp, int set )
{
    return InterlockedCompareExchange(
        (LONG volatile*)val, (LONG)set, (LONG)cmp
        );
}

static inline
void* DzAtomCasPtr( void* volatile* val, void* cmp, void* set )
{
    return (void*)InterlockedCompareExchangePointer(
        (PVOID volatile*)val, (PVOID)set, (PVOID)cmp
        );
}

#elif defined( __linux__ )

static inline
int DzAtomReadInt( int volatile* val )
{
    return *val;
}

static inline
void DzAtomSetInt( int volatile* val, int set )
{
    *val = set;
}

static inline
void* DzAtomReadPtr( void* volatile* val )
{
    return *val;
}

static inline
void DzAtomSetPtr( void* volatile* val, void* set )
{
    *val = set;
}

static inline
int DzAtomIncInt( int volatile* val )
{
    return __sync_fetch_and_add( val, 1 );
}

static inline
int DzAtomDecInt( int volatile* val )
{
    return __sync_fetch_and_sub( val, 1 );
}

static inline
int DzAtomAddInt( int volatile* val, int add )
{
    return __sync_fetch_and_add( val, add );
}

static inline
int DzAtomSubInt( int volatile* val, int sub )
{
    return __sync_fetch_and_sub( val, sub );
}

static inline
int DzAtomOrInt( int volatile* val, int mask )
{
    return __sync_fetch_and_or( val, mask );
}

static inline
int DzAtomAndInt( int volatile* val, int mask )
{
    return __sync_fetch_and_and( val, mask );
}

static inline
int DzAtomCasInt( int volatile* val, int cmp, int set )
{
    return __sync_val_compare_and_swap( val, cmp, set );
}

static inline
void* DzAtomCasPtr( void* volatile* val, void* cmp, void* set )
{
    return __sync_val_compare_and_swap( val, cmp, set );
}

#endif

static inline
void DzSpinLock( int volatile* lock )
{
    while( DzAtomCasInt( lock, 0, 1 ) == 1 );
}

static inline
void DzSpinUnlock( int volatile* lock )
{
    DzAtomSetInt( lock, 0 );
}

#ifdef __cplusplus
}
#endif

#endif // __DzCrossOs_h__