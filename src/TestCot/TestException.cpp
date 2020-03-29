
#include "CotTestUtil.h"

static int gCount = 0;

struct TestExpData
{
    int     a;
    int     b;
    int     x;
    int     y;
};

class TestClass
{
public:
    TestClass(int& val)
        : _val(val)
    {
        ++_val;
    }

    ~TestClass()
    {
        _val += 2;
    }

private:
    int&    _val;
};

void GenerateCppExp(int type, int& val)
{
    TestClass a(val);
    if (type == 1) {
        throw int(8);
    } else if (type == 2) {
        throw short(9);
    } else {
        TestExpData e;
        e.a = 1;
        e.b = 2;
        e.x = 3;
        e.y = 4;
        throw e;
    }
}

void HandleCppExpFunc(int delay, TestExpData* data, int type, int& val)
{
    try{
        TestClass b(val);
        DzSleep(delay);
        GenerateCppExp(type, val);
    }catch(TestExpData& e) {
        data->a = e.a;
        data->b = e.b;
        data->x = e.x;
        data->y = e.y;
    }catch(int e) {
        data->a = e;
    }catch(...) {
        data->b = 5;
    }
}

CotEntry CppExpRoutine(intptr_t context)
{
    gCount++;
    int delay = (int)context;
    int n = 1;

    TestExpData data;
    data.a = 0;
    data.b = 0;
    data.x = 0;
    data.y = 0;
    HandleCppExpFunc(delay, &data, 1, n);
    DZ_EXPECT_EQ(8, data.a);
    DZ_EXPECT_EQ(0, data.b);
    DZ_EXPECT_EQ(0, data.x);
    DZ_EXPECT_EQ(0, data.y);
    DZ_EXPECT_EQ(7, n);

    data.a = 0;
    data.b = 0;
    data.x = 0;
    data.y = 0;
    HandleCppExpFunc(delay, &data, 2, n);
    DZ_EXPECT_EQ(0, data.a);
    DZ_EXPECT_EQ(5, data.b);
    DZ_EXPECT_EQ(0, data.x);
    DZ_EXPECT_EQ(0, data.y);
    DZ_EXPECT_EQ(13, n);

    data.a = 0;
    data.b = 0;
    data.x = 0;
    data.y = 0;
    HandleCppExpFunc(delay, &data, 3, n);
    DZ_EXPECT_EQ(1, data.a);
    DZ_EXPECT_EQ(2, data.b);
    DZ_EXPECT_EQ(3, data.x);
    DZ_EXPECT_EQ(4, data.y);
    DZ_EXPECT_EQ(19, n);
}

CotEntry TestCppException(intptr_t context)
{
    gCount = 0;
    int n = (int)context;
    int delay = 10;
    DzHandle evt[16];
    for (int i = 0; i < n; i++) {
        evt[i] = DzCreateManualEvt(FALSE);
        DzEvtStartCot(evt[i], CppExpRoutine, (intptr_t)delay);
        delay += 3;
    }
    DzWaitMultiSynObj(n, evt, TRUE);
    for (int i = 0; i < n; i++) {
        DzDelSynObj(evt[i]);
    }
    DZ_EXPECT_EQ(n, gCount);
}

TEST(TestException, CppException)
{
    TestCot(TestCppException, (intptr_t)10);
}

#ifdef _WIN32

void HandleExpFunc(int delay, TestExpData* data)
{
    __try{
        __try{
            DzSleep(delay);
            data->b = 10 / data->a;
            data->x = data->b;
        }__except(EXCEPTION_EXECUTE_HANDLER) {
            data->y = 5;
        }
    }__finally{
        data->y += 99;
    }
}

void ContinueExpFunc(int delay, TestExpData* data)
{
    __try{
        __try{
            DzSleep(delay);
            data->b = 10 / data->a;
            data->x = data->b;
        }__except(data->a = 5, data->y = 5, EXCEPTION_CONTINUE_EXECUTION) {
        }
    }__finally{
        data->y += 99;
    }
}

CotEntry SehExpRoutine(intptr_t context)
{
    gCount++;
    int delay = (int)context;

    TestExpData data;
    data.a = 0;
    data.b = 0;
    data.x = 0;
    data.y = 0;
    HandleExpFunc(delay, &data);
    DZ_EXPECT_EQ(0, data.x);
    DZ_EXPECT_EQ(104, data.y);

    data.a = 0;
    data.b = 0;
    data.x = 0;
    data.y = 0;
    ContinueExpFunc(delay, &data);
    DZ_EXPECT_EQ(2, data.b);
    DZ_EXPECT_EQ(2, data.x);
    DZ_EXPECT_EQ(104, data.y);
}

CotEntry TestWinSehException(intptr_t context)
{
    gCount = 0;
    int n = (int)context;
    int delay = 10;
    DzHandle evt[16];
    for (int i = 0; i < n; i++) {
        evt[i] = DzCreateManualEvt(FALSE);
        DzEvtStartCot(evt[i], SehExpRoutine, (intptr_t)delay);
        delay += 3;
    }
    DzWaitMultiSynObj(n, evt, TRUE);
    for (int i = 0; i < n; i++) {
        DzDelSynObj(evt[i]);
    }
    DZ_EXPECT_EQ(n, gCount);
}

TEST(TestException, WinSehException)
{
    TestCot(TestWinSehException, (intptr_t)8);
}

#endif