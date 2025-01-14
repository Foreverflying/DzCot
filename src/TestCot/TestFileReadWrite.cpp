
#include "CotTestUtil.h"
#include "md5.h"

static int gCheck = 0;

CotEntry HelpCheckAsynTimerEntry(intptr_t context)
{
    gCheck++;
}

ssize_t ReadFileFull(int fd, char* buff, size_t count)
{
    ssize_t tmp = 0;
    ssize_t ret = 0;
    do {
        ret += tmp;
        tmp = DzRead(fd, buff + ret, count - ret);
    } while (tmp > 0 && ret < (ssize_t)count);
    return tmp < 0 ? -1 : ret;
}

ssize_t WriteFileFull(int fd, const char* buff, size_t count)
{
    ssize_t tmp = 0;
    ssize_t ret = 0;
    do {
        ret += tmp;
        tmp = DzWrite(fd, buff + ret, count - ret);
    } while (tmp > 0 && ret < (ssize_t)count);
    return tmp < 0 ? -1 : ret;
}

void ReadFileEntry(char* buff, size_t buffLen)
{
    unsigned char md5Ret[16];
    md5_context mc;

    int fd = DzOpenT(_T("../test/Sailing.mp3"), DZ_O_RD);
    DZ_ASSERT_NE(-1, fd);
    size_t n = DzFileSize(fd);
    DZ_ASSERT_EQ(3678906, n);
    DZ_ASSERT_GE(buffLen, n);

    unsigned char md5a[] = {
        0x81, 0x1B, 0x87, 0xB5, 0xA1, 0xDC, 0x52, 0x97,
        0x7A, 0x2D, 0xFF, 0x50, 0x4A, 0xC1, 0xCA, 0xBD
    };
    size_t count = ReadFileFull(fd, buff, n);
    DZ_ASSERT_EQ(n, count);

    md5_starts(&mc);
    md5_update(&mc, (uint8*)buff, (uint32)count);
    md5_finish(&mc, md5Ret);
    int ret = memcmp(md5Ret, md5a, 16);
    DZ_ASSERT_EQ(0, ret);

    unsigned char md5b[] = {
        0x09, 0xA5, 0x06, 0x9C, 0xA7, 0xB5, 0x13, 0x90,
        0x90, 0x68, 0x36, 0xA6, 0xEA, 0x55, 0x1D, 0x48
    };
    size_t seekHead = 0x296040;
    size_t seekEnd = 0x29806b;
    size_t seekLen = seekEnd - seekHead;
    size_t seekRet = DzSeek(fd, seekHead, DZ_SEEK_SET);
    DZ_ASSERT_EQ(seekHead, seekRet);
    count = ReadFileFull(fd, buff, seekLen);
    DZ_ASSERT_EQ(seekLen, count);

    md5_starts(&mc);
    md5_update(&mc, (uint8*)buff, (uint32)count);
    md5_finish(&mc, md5Ret);
    ret = memcmp(md5Ret, md5b, 16);
    DZ_ASSERT_EQ(0, ret);

    seekRet = DzSeek(fd, 0, DZ_SEEK_CUR);
    DZ_ASSERT_EQ(seekEnd, seekRet);

    unsigned char content[] = {
        0xff, 0xfb, 0x82, 0x00, 0xff, 0x80, 0x0a, 0x50,
        0x00, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    seekHead = 0x382200;
    seekLen = n - seekHead;
    seekRet = DzSeek(fd, - (ssize_t)seekLen, DZ_SEEK_END);
    count = ReadFileFull(fd, buff, 16);
    DZ_ASSERT_EQ(16, count);
    ret = memcmp(buff, content, 16);
    DZ_ASSERT_EQ(0, ret);

    DzClose(fd);
}

CotEntry TestReadFile(intptr_t context)
{
    gCheck = 0;
    DzHandle checkTimer = DzCreateCallbackTimer(10, TRUE, HelpCheckAsynTimerEntry);
    size_t buffLen = 4 * 1024 * 1024;
    char *buff = (char*)DzMalloc(buffLen);

    ReadFileEntry(buff, buffLen);
    DzDelCallbackTimer(checkTimer);
    // printf("gCheck : %d\r\n", gCheck);
    // ASSERT_NE(0, gCheck);

    DzFree(buff);
}

void WriteFileEntry(char* buff, size_t buffLen)
{
    char j = 'a';
    for (int i = 0; i < (int)buffLen; i++) {
        buff[i] = j++;
        if (j == 'z') {
            j = 'A';
        } else if (j == 'Z') {
            j = 'a';
        }
    }

    int fd = DzOpenT(
        _T("../test/test_write.txt"),
        DZ_O_RDWR | DZ_O_TRUNC | DZ_O_CREATE
        );
    DZ_ASSERT_NE(-1, fd);

    size_t count = WriteFileFull(fd, buff, buffLen);
    DZ_ASSERT_EQ(buffLen, count);

    char tmpBuff[] = {
        '_', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '_'
    };
    size_t tmpLen = sizeof(tmpBuff);
    size_t seekRet = DzSeek(fd, 256, DZ_SEEK_SET);
    count = WriteFileFull(fd, tmpBuff, tmpLen);
    DZ_ASSERT_EQ(seekRet, 256);
    DZ_ASSERT_EQ(tmpLen, count);

    seekRet = DzSeek(fd, 32, DZ_SEEK_CUR);
    count = WriteFileFull(fd, tmpBuff, tmpLen);
    DZ_ASSERT_EQ(seekRet, 300);
    DZ_ASSERT_EQ(tmpLen, count);

    seekRet = DzSeek(fd, -32, DZ_SEEK_END);
    count = WriteFileFull(fd, tmpBuff, tmpLen);
    DZ_ASSERT_EQ(seekRet, 4194272);
    DZ_ASSERT_EQ(tmpLen, count);

    seekRet = DzSeek(fd, 0, DZ_SEEK_SET);
    count = ReadFileFull(fd, buff, buffLen);
    DZ_ASSERT_EQ(seekRet, 0);
    DZ_ASSERT_EQ(buffLen, count);

    unsigned char md5[] = {
        0xB4, 0x49, 0x72, 0xAE, 0x2C, 0x5E, 0x2B, 0xDF,
        0x5D, 0xC8, 0x1C, 0xEB, 0x26, 0x77, 0xEF, 0x40
    };
    unsigned char md5Ret[16];
    md5_context mc;
    md5_starts(&mc);
    md5_update(&mc, (uint8*)buff, (uint32)count);
    md5_finish(&mc, md5Ret);
    int ret = memcmp(md5Ret, md5, 16);
    DZ_ASSERT_EQ(0, ret);

    DzClose(fd);
}

CotEntry TestWriteFile(intptr_t context)
{
    gCheck = 0;
    DzHandle checkTimer = DzCreateCallbackTimer(10, TRUE, HelpCheckAsynTimerEntry);
    size_t buffLen = 4 * 1024 * 1024;
    char* buff = (char*)DzMalloc(buffLen);

    WriteFileEntry(buff, buffLen);
    DzDelCallbackTimer(checkTimer);
    // printf("gCheck : %d\r\n", gCheck);
    // ASSERT_NE(0, gCheck);

    DzFree(buff);
}

TEST(TestFileReadWrite, ReadFile)
{
    // TestCot(TestReadFile);
}

TEST(TestFileReadWrite, WriteFile)
{
    // TestCot(TestWriteFile);
}
