/********************************************************************
    created:    2010/02/11 22:12
    file:       DzCoreLnx.c
    author:     Foreverflying
    purpose:    
*********************************************************************/

#include "../DzIncOs.h"
#include "../DzCoreOs.h"
#include "../DzCore.h"

void InitAsynIo( DzAsynIo *asynIo )
{
    InitFastEvt( &asynIo->inEvt );
    InitFastEvt( &asynIo->outEvt );
    asynIo->inEvt.dzThread = NULL;
    asynIo->outEvt.dzThread = NULL;
}

void InitDzThread( DzThread *dzThread, int sSize )
{

}

// DzcotRoutine:
// the real entry the co thread starts, it call the user entry
// after that, the thread is finished, so put itself to the thread pool
// schedule next thread
void __stdcall DzcotRoutine( DzRoutine entry, void* context )
{
    DzHost *host = GetHost();
	while(1){
		//call the entry
		( *(DzRoutine volatile *)(&entry) )( *(void* volatile *)(&context) );

		//free the thread
		host->threadCount--;
		FreeDzThread( host, host->currThread );

		//then schedule another thread
		Schedule( host );
	}
}
