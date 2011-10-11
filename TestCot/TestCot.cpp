// TestCot.cpp : Defines the entry point for the console application.
//

#include "CotTestUtil.h"

int main(int argc, _TCHAR* argv[])
{
    int ret = MainEntry( argc, argv );
    getchar();
    return ret;
}

int _tmain(int argc, _TCHAR* argv[])
{
    int ret = MainEntry( argc, argv );
    getchar();
    return ret;
}
