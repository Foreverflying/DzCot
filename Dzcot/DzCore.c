#include "DzIncOs.h"
#include "DzCore.h"

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
