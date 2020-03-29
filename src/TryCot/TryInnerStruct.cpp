
#include "CotTryUtil.h"

namespace Inner{
#undef DZ_MAX_IOV
#undef DZ_MAX_HOST
#include "../DzCot/DzStructs.h"
#include "../DzCot/DzBase.h"
}

void TryInnerStruct()
{
    printf("Host size is %d\r\n", (int)sizeof(Inner::DzHost));
    printf("DzCot size is %d\r\n", (int)sizeof(Inner::DzCot));
}