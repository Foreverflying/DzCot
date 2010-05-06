
#ifndef _DZ_CORE_WIN_H_
#define _DZ_CORE_WIN_H_

#include "../DzStructs.h"
#include "../../DzcotData/DzcotData.h"

#define PAGE_SIZE                   4096
#define DZ_STACK_UNIT_SIZE          65536
#define STORE_HOST_IN_ARBITRARY_USER_POINTER

#ifdef __cplusplus
extern "C"{
#endif

char* AllocStack( int sSize );
void FreeStack( char *stack, int sSize );
char* CommitStack( char *stack, size_t size );
void DeCommitStack( char *stack, char *stackLimit );
void __stdcall DzcotRoutine( DzRoutine entry, void* context );
void __fastcall DzSwitch( DzHost *host, DzThread *dzThread );

#if defined( _X86_ )

struct DzStackBottom
{
    void*       stackLimit;
    void*       stackPtr;
    void*       exceptPtr;
    void*       unusedEdi;
    void*       unusedEsi;
    void*       unusedEbx;
    void*       unusedEbp;
    void*       routineEntry;
    void*       unusedEip;
    DzRoutine   entry;
    void*       context;
};

#define DzcotRoutineEntry DzcotRoutine

#elif defined( _M_AMD64 )

struct DzStackBottom
{
    void*       stackLimit;
    void*       stackPtr;
    void*       unusedR15;
    void*       unusedR14;
    void*       unusedR13;
    void*       unusedR12;
    void*       unusedRdi;
    void*       unusedRsi;
    void*       unusedRbx;
    void*       unusedRbp;
    void*       routineEntry;
    void*       unusedEip;
    DzRoutine   entry;
    void*       context;
    void*       unusedR8;
    void*       unusedR9;
};

void CallDzcotRoutine();

#define DzcotRoutineEntry CallDzcotRoutine

#endif

inline void DzInitCot( DzHost *host, DzThread *dzThread )
{
    struct DzStackBottom *bottom;

    bottom = ( (struct DzStackBottom*)dzThread->stack ) - 1;
    bottom->routineEntry = DzcotRoutineEntry;
#ifdef _X86_
    bottom->exceptPtr = host->originExceptPtr;
#endif
    bottom->stackPtr = dzThread->stack;
    bottom->stackLimit = dzThread->stackLimit;

    dzThread->sp = bottom;

    /*
    __asm{
        mov ecx, dzThread

        //eax = host
        mov eax, host

        //temp change esp
        mov edx, esp
        mov esp, [ecx] DzThread.stack

        //push host
        mov [esp-12], eax

        //keep the space for context, entry
        //keep the host's space
        //and keep two parameter space for __stdcall SwitchToCot
        sub esp, 20

        //push the next eip
        push call_p

        //the ScheduleThread caller's ebp, ignored
        //ebx, esi, edi, ignored
        sub esp, 16

        //store cot info to stack, dzThread->esp = esp
        push [eax] DzHost.originExceptPtr
        push [ecx] DzThread.stack
        push [ecx] DzThread.stackLimit
        mov [ecx] DzThread.esp, esp

        //restore the esp
        mov esp, edx
    }
    return;
call_p:
    __asm{ call DzcotRoutine }
    */
}

inline void SetThreadEntry( DzThread *dzThread, DzRoutine entry, void *context )
{
    struct DzStackBottom *bottom;

    bottom = ( (struct DzStackBottom*)dzThread->stack ) - 1;
    bottom->entry = entry;
    bottom->context = context;

    /*
    __asm{
        mov ecx, dzThread
            mov eax, [ecx] DzThread.stack
            mov edx, context;
        mov [eax-4], edx
            mov edx, entry
            mov [eax-8], edx
    }
    */
}

inline void* GetExceptPtr()
{
#if defined( _X86_ )
    return (void*)__readfsdword( 0 );
#elif defined( _M_AMD64 )
    return NULL;
#endif
}

inline void InitTlsIndex()
{
#ifdef STORE_HOST_IN_ARBITRARY_USER_POINTER
#else
    if( tlsIndex == TLS_OUT_OF_INDEXES ){
        while( InterlockedExchange( &tlsLock, 1 ) == 1 );
        if( tlsIndex == TLS_OUT_OF_INDEXES ){
            tlsIndex = TlsAlloc();
        }
        InterlockedExchange( &tlsLock, 0 );
    }
#endif
}

inline DzHost* GetHost()
{
#ifdef STORE_HOST_IN_ARBITRARY_USER_POINTER
#if defined( _X86_ )
    return (DzHost*)__readfsdword( 20 );
#elif defined( _M_AMD64 )
    return *(DzHost**)(__readgsqword( 0x30 ) + 40);
#endif
#else
    return (DzHost*)TlsGetValue( tlsIndex );
#endif
}

inline void SetHost( DzHost *host )
{
#ifdef STORE_HOST_IN_ARBITRARY_USER_POINTER
#if defined( _X86_ )
    __writefsdword( 20, (DWORD)host );
#elif defined( _M_AMD64 )
    *(DzHost**)(__readgsqword( 0x30 ) + 40) = host;
#endif
#else
    TlsSetValue( tlsIndex, host );
#endif
}

#ifdef __cplusplus
};
#endif

#endif  //#ifndef _DZ_CORE_WIN_H_