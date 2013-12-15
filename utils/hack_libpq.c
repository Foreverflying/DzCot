/**
 *  @file       hack_libpq.c
 *  @brief      
 *  @author     Foreverflying <foreverflying@live.cn>
 *  @date       2013/12/14
 *
 */

#include "libpq-int.h"
#include "hack_libpq.h"

#ifdef __DZCOT_HACKED_LIBPQ

#undef pqSocketCheck
#undef pqReadData
#undef pqSendSome

typedef union _Helper Helper;

union _Helper
{
    struct{
        union
        {
            Helper*         next;
            struct
            {
                int         sendLen;
                int         recvLen;
            };
        };
        PGconn*             conn;
        DzHandle            sendEvt;
        DzHandle            recvEvt;
    };
    int                     padding[8];
};

static Helper* _helperPool = NULL;

Helper* NewHelper()
{
    int i;
    Helper* ret;

    if( !_helperPool ){
        _helperPool = (Helper*)DzAllocPermanentChunk(
            sizeof(Helper) * 32
            );
        for( i = 0; i < 32; i++ ){
            _helperPool[i].next = _helperPool + i + 1;
        }
        _helperPool[31].next = NULL;
    }
    ret = _helperPool;
    _helperPool = _helperPool->next;
    ret->recvEvt = DzCreateManualEvt( TRUE );
    ret->sendEvt = DzCreateManualEvt( TRUE );
    ret->recvLen = 0;
    ret->sendLen = 0;
    return ret;
}

void DelHelper( Helper* helper )
{
    DzDelSynObj( helper->recvEvt );
    DzDelSynObj( helper->sendEvt );
    helper->next = _helperPool;
    _helperPool = helper;
}

int PqSocket( int domain, int type, int protocol )
{
    int fd = DzSocket( domain, type, protocol );
    if( fd >= 0 ){
        DzSetFdData( fd, (intptr_t)NewHelper() );
    }
    return fd;
}

int PqClose( int fd )
{
    Helper* helper = (Helper*)DzGetFdData( fd );
    DelHelper( helper );
    return DzClose( fd );
}

#ifdef USE_SSL

CotEntry RecvEntry( intptr_t context )
{

}

CotEntry SendEntry( intptr_t context )
{

}

#else

CotEntry RecvEntry( intptr_t context )
{
    Helper* helper = (Helper*)context;
    PGconn* conn = helper->conn;
    int len = DzRecv(
        conn->sock,
        conn->inBuffer + conn->inEnd,
        conn->inBufSize - conn->inEnd,
        0
        );
    helper->recvLen = len > 0 ? len : -1;
}

CotEntry SendEntry( intptr_t context )
{
    Helper* helper = (Helper*)context;
    PGconn* conn = helper->conn;
    int len = DzSend(
        conn->sock,
        conn->outBuffer,
        conn->outCount,
        0
        );
    helper->sendLen = len >= 0 ? len : -1;
}

#endif  //if USE_SSL

int pqSocketCheck(PGconn *conn, int forRead, int forWrite, time_t end_time)
{
    int ret;
    int timeout;
    time_t now;
    int fd = conn->sock;
    Helper* helper = (Helper*)DzGetFdData( fd );
    helper->conn = conn;
    timeout = -1;
    if( end_time != (time_t)-1 ){
        now = time(NULL);
        timeout = end_time > now ? (int)( end_time - now ) * 1000 : 0;
    }
    if( forRead ){
        if( !helper->recvLen && DzIsNotified( helper->recvEvt ) ){
            /* Left-justify any data in the buffer to make room */
            if( conn->inStart < conn->inEnd ){
                if( conn->inStart > 0 ){
                    memmove(
                        conn->inBuffer,
                        conn->inBuffer + conn->inStart,
                        conn->inEnd - conn->inStart
                        );
                    conn->inEnd -= conn->inStart;
                    conn->inStart = 0;
                }
            }else{
                /* buffer is logically empty, reset it */
                conn->inStart = conn->inCursor = conn->inEnd = 0;
            }
            if( conn->inBufSize - conn->inEnd < 8192 ){
                if( pqCheckInBufferSpace( conn->inEnd + 8192, conn ) ){
                    if( conn->inBufSize - conn->inEnd < 100 ){
                        return -1;
                    }
                }
            }
            DzResetEvt( helper->recvEvt );
            DzEvtStartCotInstant(
                helper->recvEvt,
                RecvEntry,
                (intptr_t)helper,
                CP_NORMAL,
                ST_UM
                );
        }
        if( !helper->recvLen && !forWrite ){
            if( timeout == 0 ){
                return 0;
            }
            ret = DzWaitSynObj( helper->recvEvt, timeout );
            if( ret < 0 ){
                return 0;
            }
            return helper->recvLen > 0 ? 1 : -1;
        }
        if( !forWrite ){
            return helper->recvLen > 0 ? 1 : -1;
        }
    }
    if( forWrite && conn->outCount ){
        if( !helper->sendLen && DzIsNotified( helper->sendEvt ) ){
            DzResetEvt( helper->sendEvt );
            DzEvtStartCotInstant(
                helper->sendEvt,
                SendEntry,
                (intptr_t)helper,
                CP_NORMAL,
                ST_UM
                );
        }
        if( !helper->sendLen && !forRead ){
            if( timeout == 0 ){
                return 0;
            }
            ret = DzWaitSynObj( helper->sendEvt, timeout );
            if( ret < 0 ){
                return 0;
            }
            return helper->sendLen >= 0 ? 1 : -1;
        }
        if( !forRead ){
            return helper->sendLen > 0 ? 1 : -1;
        }
    }
    if( !helper->recvLen && !helper->sendLen && conn->outCount ){
        if( timeout == 0 ){
            return 0;
        }
        ret = DzWaitMultiSynObj( 2, &helper->sendEvt, FALSE, timeout );
        if( ret < 0 ){
            return 0;
        }
    }
    if( helper->recvLen < 0 || helper->sendLen < 0 ){
        return -1;
    }else if( helper->recvLen && ( helper->sendLen || !conn->outCount ) ){
        return 2;
    }else{
        return 1;
    }
}

int pqReadData( PGconn* conn )
{
    Helper* helper;
    if( conn->sock < 0 ){
        printfPQExpBuffer(
            &conn->errorMessage,
            libpq_gettext( "connection not open\n" )
            );
        return -1;
    }
    helper = (Helper*)DzGetFdData( conn->sock );
    if( helper->recvLen < 0 ){
        printfPQExpBuffer(
            &conn->errorMessage,
            libpq_gettext(
                "server closed the connection unexpectedly\n"
                "\tThis probably means the server terminated abnormally\n"
                "\tbefore or while processing the request.\n"
                )
            );
        pqDropConnection(conn);
        conn->status = CONNECTION_BAD;		/* No more connection to backend */
        return -1;
    }
    conn->inEnd += helper->recvLen;
    helper->recvLen = 0;
    return 1;
}

int pqSendSome( PGconn* conn, int len )
{
    Helper* helper;
    int result = 0;
    int remaining = conn->outCount;
    if( conn->sock < 0 ){
        printfPQExpBuffer(
            &conn->errorMessage,
            libpq_gettext( "connection not open\n" )
            );
        return -1;
    }
    helper = (Helper*)DzGetFdData( conn->sock );
    while( len > 0 ){
        if( helper->sendLen == 0 ){
            if( pqWait( 0, 1, conn ) ){
                return -1;
            }
        }
        if( helper->sendLen < 0 ){
            conn->outCount = 0;
            return -1;
        }
        if( helper->sendLen >= len ){
            helper->sendLen -= len;
            remaining -= len;
            len = 0;
        }else{
            helper->sendLen = 0;
            remaining -= helper->sendLen;
            len -= helper->sendLen;
        }
        if( len > 0 ){
            if( pqReadData( conn ) < 0 ){
                result = -1;
                break;
            }
            if( pqWait( TRUE, TRUE, conn ) ){
                result = -1;
                break;
            }
        }
    }
    if( remaining > 0 ){
        memmove(
            conn->outBuffer,
            conn->outBuffer + conn->outCount - remaining,
            remaining
            );
    }
    conn->outCount = remaining;
    return result;
}

int pq_pthread_mutex_init( DzHandle* mp, MtxAttr attr )
{
    *mp = NULL;
    return 0;
}

int pq_pthread_mutex_lock( DzHandle* mp )
{
    if( !*mp ){
        *mp = DzCreateMtx( TRUE );
    }else{
        DzWaitSynObj( *mp, -1 );
    }
    return 0;
}

int pq_pthread_mutex_unlock( DzHandle* mp )
{
    DzReleaseMtx( *mp );
    if( DzIsNotified( *mp ) ){
        DzDelSynObj( *mp );
        *mp = NULL;
    }
    return 0;
}

#ifdef WIN32

#undef WSAIoctl

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
    )
{
    return WSAIoctl(
        (SOCKET)DzRawSocket( s ),
        dwIoControlCode,
        lpvInBuffer,
        cbInBuffer,
        lpvOutBuffer,
        cbOutBuffer,
        lpcbBytesReturned,
        lpOverlapped,
        lpCompletionRoutine
        );
}

#elif defined __linux__

#undef fcntl

int Pq_fcntl( int fd, int cmd, int arg )
{
    return fcntl( (int)DzRawSocket( fd ), cmd, arg );
}

#endif

#endif  //ifdef __DZCOT_HACKED_LIBPQ
