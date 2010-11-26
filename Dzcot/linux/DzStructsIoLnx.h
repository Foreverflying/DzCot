/********************************************************************
    created:    2010/02/11 22:05
    file:       DzStructsIoLnx.h
    author:     Foreverflying
    purpose:    
*********************************************************************/

#ifndef __DzStructsIoLnx_h__
#define __DzStructsIoLnx_h__

#include "../DzStructs.h"

#define ASYNIO_SIGN_ERR_MASK     0xffff
#define ASYNIO_SIGN_ISFILE       0x80000000

typedef struct _DzAsynIo
{
	union{
	    DzQItr      qItr;
	    size_t      sign;
	};
    DzFastEvt       inEvt;
    DzFastEvt       outEvt;
}DzAsynIo;

#endif // __DzStructsIoLnx_h__
