#ifdef TEST

#ifdef __cplusplus
extern "C" {
#endif
#include "unity.h"
//#include "wave_gen.h"
#include <string.h>
#ifdef __cplusplus
}
#endif
void setUp(void)
{
}

void tearDown(void)
{
}

void test_wave_gen_low_frew(void){
    TEST_ASSERT_NOT_EQUAL_INT8(-1, 0);
}
#endif