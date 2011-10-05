
#include "stdafx.h"
#include <gtest/gtest.h>
#include "Util.h"
#include "md5.h"

static int gCheck = 0;

void __stdcall HelpCheckAsynTimerEntry( void* context )
{
    gCheck++;
}

void ReadFileEntry( char* buff, int buffLen )
{
    unsigned char md5Ret[16];
    md5_context mc;

    int fd = DzOpenFile( _T( "../../test/Sailing.mp3" ), DZ_O_RD );
    ASSERT_NE( -1, fd );
    size_t n = DzGetFileSize( fd );
    ASSERT_EQ( 3678906, n );
    ASSERT_GE( buffLen, (int)n );

    unsigned char md5a[] = {
        0x81, 0x1B, 0x87, 0xB5, 0xA1, 0xDC, 0x52, 0x97,
        0x7A, 0x2D, 0xFF, 0x50, 0x4A, 0xC1, 0xCA, 0xBD
    };
    size_t count = DzReadFile( fd, buff, n );
    ASSERT_EQ( n, count );

    md5_starts( &mc );
    md5_update( &mc, (uint8*)buff, count );
    md5_finish( &mc, md5Ret );
    int ret = memcmp( md5Ret, md5a, 16 );
    ASSERT_EQ( 0, ret );

    unsigned char md5b[] = {
        0x09, 0xA5, 0x06, 0x9C, 0xA7, 0xB5, 0x13, 0x90,
        0x90, 0x68, 0x36, 0xA6, 0xEA, 0x55, 0x1D, 0x48
    };
    size_t seekHead = 0x296040;
    size_t seekEnd = 0x29806b;
    ssize_t seekLen = seekEnd - seekHead;
    size_t seekRet = DzSeekFile( fd, seekHead, DZ_SEEK_SET );
    ASSERT_EQ( seekHead, seekRet );
    count = DzReadFile( fd, buff, seekLen );
    ASSERT_EQ( seekLen, count );

    md5_starts( &mc );
    md5_update( &mc, (uint8*)buff, count );
    md5_finish( &mc, md5Ret );
    ret = memcmp( md5Ret, md5b, 16 );
    ASSERT_EQ( 0, ret );

    seekRet = DzSeekFile( fd, 0, DZ_SEEK_CUR );
    ASSERT_EQ( seekEnd, seekRet );

    unsigned char content[] = {
        0xff, 0xfb, 0x82, 0x00, 0xff, 0x80, 0x0a, 0x50,
        0x00, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    seekHead = 0x382200;
    seekLen = n - seekHead;
    seekRet = DzSeekFile( fd, - seekLen, DZ_SEEK_END );
    count = DzReadFile( fd, buff, 16 );
    ASSERT_EQ( 16, count );
    ret = memcmp( buff, content, 16 );
    ASSERT_EQ( 0, ret );

    DzCloseFile( fd );
}

void __stdcall TestReadFile( void* context )
{
    gCheck = 0;
    DzHandle checkTimer = DzCreateCallbackTimer( 10, 0, HelpCheckAsynTimerEntry );
    int buffLen = 4 * 1024 * 1024;
    char *buff = new char[ buffLen ];

    ReadFileEntry( buff, buffLen );
    //printf( "gCheck : %d\r\n", gCheck );
    DzCloseCallbackTimer( checkTimer );
    ASSERT_NE( 0, gCheck );
}

void WriteFileEntry( char* buff, int buffLen )
{
    char j = 'a';
    for( int i = 0; i < buffLen; i++ ){
        buff[i] = j++;
        if( j == 'z' ){
            j = 'A';
        }else if( j == 'Z' ){
            j = 'a';
        }
    }

    int fd = DzOpenFile(
        _T( "../../test/test_write.txt" ),
        DZ_O_RDWR | DZ_O_TRUNC | DZ_O_CREATE
        );
    ASSERT_NE( -1, fd );
    size_t n = DzGetFileSize( fd );
    ASSERT_EQ( 0, n );
    size_t count = DzWriteFile( fd, buff, buffLen );
    ASSERT_EQ( buffLen, count );

    char tmpBuff[] = {
        '_', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '_'
    };
    int tmpLen = sizeof( tmpBuff );
    size_t seekRet = DzSeekFile( fd, 256, DZ_SEEK_SET );
    count = DzWriteFile( fd, tmpBuff, tmpLen );
    ASSERT_EQ( tmpLen, count );

    seekRet = DzSeekFile( fd, 32, DZ_SEEK_CUR );
    count = DzWriteFile( fd, tmpBuff, tmpLen );
    ASSERT_EQ( tmpLen, count );

    seekRet = DzSeekFile( fd, -32, DZ_SEEK_END );
    count = DzWriteFile( fd, tmpBuff, tmpLen );
    ASSERT_EQ( tmpLen, count );

    DzSeekFile( fd, 0, DZ_SEEK_SET );
    count = DzReadFile( fd, buff, buffLen );
    ASSERT_EQ( buffLen, count );

    unsigned char md5[] = {
        0xB4, 0x49, 0x72, 0xAE, 0x2C, 0x5E, 0x2B, 0xDF,
        0x5D, 0xC8, 0x1C, 0xEB, 0x26, 0x77, 0xEF, 0x40
    };
    unsigned char md5Ret[16];
    md5_context mc;
    md5_starts( &mc );
    md5_update( &mc, (uint8*)buff, count );
    md5_finish( &mc, md5Ret );
    int ret = memcmp( md5Ret, md5, 16 );
    ASSERT_EQ( 0, ret );

    DzCloseFile( fd );
}

void __stdcall TestWriteFile( void* context )
{
    gCheck = 0;
    DzHandle checkTimer = DzCreateCallbackTimer( 10, 0, HelpCheckAsynTimerEntry );
    int buffLen = 4 * 1024 * 1024;
    char* buff = new char[ buffLen ];

    WriteFileEntry( buff, buffLen );

    ASSERT_NE( 0, gCheck );
    //printf( "gCheck : %d\r\n", gCheck );
    DzCloseCallbackTimer( checkTimer );
}

TEST( TestFileReadWrite, ReadFile )
{
    TestCot( TestReadFile );
}

TEST( TestFileReadWrite, WriteFile )
{
    TestCot( TestWriteFile );
}