/********************************************************************
    created:    2011/10/06 20:58
    file:       DzDebug.c
    author:     Foreverflying
    purpose:    
********************************************************************/

#include "DzDebug.h"
#include "DzStructsDebug.h"
#include "DzStructs.h"
#include "DzBase.h"

#ifdef __DBG_DEBUG_CHECK_MODE

#define DEBUG_CHECK_MARK    0xfefefefe

int __DbgGetLastErr( DzHost* host )
{
    return __DbgPtr( host->currCot )->lastErr;
}

void __DbgSetLastErr( DzHost* host, int err )
{
    __DbgPtr( host->currCot )->lastErr = err;
}

void __DbgMarkCurrStackForCheck( DzHost* host )
{
    DzCot* dzCot;
    int* begin;
    int* end;
    int size;

    dzCot = GetHost()->currCot;
    size = host->cotStackSize[ dzCot->sType ];
    end = (int*)( dzCot->stack - size );
    if( size >= DZ_MIN_PAGE_STACK_SIZE ){
        end += CHECK_RESERV_SIZE / sizeof( int );
    }
    for( begin = &size - 64; begin >= end; begin-- ){
        *begin = DEBUG_CHECK_MARK;
    }
    __DbgPtr( dzCot )->maxStkUse = (int)( begin + 1 - end );
}

void __DbgCheckCotStackOverflow( DzHost* host, DzCot* dzCot )
{
    int tmp;
    int* begin;
    int* end;
    __DbgPtrType( DzCot ) p = __DbgPtr( dzCot );

    if( dzCot == &host->centerCot ){
        return;
    }
    tmp = host->cotStackSize[ dzCot->sType ];
    begin = (int*)dzCot->stack - p->maxStkUse - 1;
    end = (int*)( dzCot->stack - tmp );
    if( tmp >= DZ_MIN_PAGE_STACK_SIZE ){
        end += CHECK_RESERV_SIZE / sizeof( int );
    }
    //*
    if( *end != DEBUG_CHECK_MARK ){
        p->maxStkUse = tmp / ( tmp & 1 );
    }
    for( tmp = 0; end < begin; end++ ){
        if( *end != DEBUG_CHECK_MARK ){
            p->maxStkUse = (int)( (int*)dzCot->stack - end );
            if( p->maxStkUse > __DbgPtr( host )->maxStkUse[ dzCot->sType ] ){
                __DbgPtr( host )->maxStkUse[ dzCot->sType ] = p->maxStkUse;
            }
            return;
        }
        tmp++;
    }
    //*/
    /*
    for( tmp = 0; begin >= end; begin-- ){
        if( *begin == DEBUG_CHECK_MARK ){
            tmp++;
            if( tmp == 16 ){
                p->maxStkUse = (int)( (int*)dzCot->stack - begin - tmp );
                if( p->maxStkUse > __DbgPtr( host )->maxStkUse[ dzCot->sType ] ){
                    __DbgPtr( host )->maxStkUse[ dzCot->sType ] = p->maxStkUse;
                }
                return;
            }
        }else{
            tmp = 0;
        }
    }
    //rise a exception by divide 0
    p->maxStkUse = tmp / (int)( begin + 1 - end );
    //*/
}

void __DbgInitDzHost( DzHost* host )
{
    int i;

    for( i = 0; i < STACK_TYPE_COUNT; i++ ){
        __DbgPtr( host )->maxStkUse[i] = 0;
    }
}

void __DbgInitDzCot( DzHost* host, DzCot* dzCot )
{
    __DbgPtr( dzCot )->lastErr = 0;
    __DbgPtr( dzCot )->maxStkUse = 0;
}

int __DbgGetMaxStackUse( DzHost* host, int sType )
{
    return __DbgPtr( host )->maxStkUse[ sType ] * sizeof( int ); 
}

#endif