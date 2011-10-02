// TestCot.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <gtest/gtest.h>
#include "TestCompileInC.h"

int main(int argc, char* argv[])
{
    TestCompileInC();
    char* av[] = {
        "TestCot",
        //"--gtest_break_on_failure",
        //"--gtest_filter=TestCotMechanism.*",
        "--gtest_filter=TestSynObj.*",
        //"--gtest_filter=TestCallbackTimer.*",
        //"--gtest_filter=TestException.*",
        //"--gtest_filter=TestSocket.*",
        //"--gtest_filter=TestSocket.SendRecvSendRecv",
        //"--gtest_filter=TestSocket.LargeBuffer",
        //"--gtest_filter=TestFileReadWrite.*",
        ""
    };
    int ac = sizeof( av ) / sizeof( char* );
    testing::InitGoogleTest( &ac, av );

    //testing::InitGoogleTest( &argc, argv );
    int ret = RUN_ALL_TESTS();
    getchar();
    return ret;
}

int _tmain(int argc, char* argv[])
{
    return main( argc, argv );
}
