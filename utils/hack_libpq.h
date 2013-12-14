/**
 *  @file       hack_libpq.h
 *  @brief      hack the postgresSQL's connect library to use it in DzCot.
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2013/12/14
 *
 *  The libpq's source code's location:
 *      postgresSQL source folder/src/interfaces/libpq
 *
 *  to use this hack file, you need to:
 *  1)  copy Inc_DzCot.h, Inc_DzSocketReplace.h, hack_libpq.h, hack_libpq.c
 *      to this folder.
 *  2)  copy DzCot.lib to this folder.
 *  3)  open win32.mak, add following lines:
 *      append to the CLEAN segment:        -@erase "$(INTDIR)\hack_libpq.obj"
 *      append to variable LIB32_OBJS:      $(INTDIR)\hack_libpq.obj
 *      append to variable LINK32_FLAGS:    DzCot.lib
 *  4)  at the tail of "libpq-fe.h", just before the line "#ifdef __cplusplus"
 *      insert:     #include <hack_libpq.h>
 *  5)  in the file "fe-misc.c", rename three functions, just modify the
 *      function's declarations and definitions, NOT calling:
 *          pqReadData => _pqReadData
 *          pqSendSome => _pqSendSome
 *          pqSocketCheck => _pqSocketCheck
 *  6)  run nmake -f win32.mak to build the hacked windows version libpq
 *  7)  now hope you enjoy the magic :)
 */

/* comment the following macro if you want to build the original version */
/* or else you get a DzCot hacked libpq */
#define __DZCOT_HACKED_LIBPQ

#ifdef __DZCOT_HACKED_LIBPQ
#include <DzSocketReplace.h>

struct PGconn;

extern int pqSendSome( PGconn* conn, int len );
extern int pqSocketCheck( PGconn* conn, int forRead, int forWrite, time_t end );
extern int pqReadData( PGconn* conn );
extern int PqSocket( int domain, int type, int protocol );
extern int PqClose( int fd );

#undef socket
#define socket          PqSocket

#undef closesocket
#define closesocket     PqClose

#undef close
#define close           PqClose

#undef pg_set_noblock
#define pg_set_noblock( conn )      true

#undef pg_set_block
#define pg_set_block( conn )        true

#undef pthread_mutex_init
#define pthread_mutex_init pq_pthread_mutex_init

#undef pthread_mutex_lock
#define pthread_mutex_lock pq_pthread_mutex_lock

#undef pthread_mutex_unlock
#define pthread_mutex_unlock pq_pthread_mutex_unlock

#define CRITICAL_SECTION void

#ifdef WIN32

int PqWSAIoctl(
    __in   SOCKET s,
    __in   DWORD dwIoControlCode,
    __in   LPVOID lpvInBuffer,
    __in   DWORD cbInBuffer,
    __out  LPVOID lpvOutBuffer,
    __in   DWORD cbOutBuffer,
    __out  LPDWORD lpcbBytesReturned,
    __in   LPWSAOVERLAPPED lpOverlapped,
    __in   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );

#define WSAIoctl( s, code, inBuf, inSize, outBuf, outSize, retSize, ol, cr )\
    PqWSAIoctl( s, code, inBuf, inSize, outBuf, outSize, retSize, ol, cr )

#endif

#else

#define pqSocketCheck       _pqSocketCheck
#define pqReadData          _pqReadData
#define pqSendSome          _pqSendSome

#endif  //ifdef __DZCOT_HACKED_LIBPQ
