#ifdef TEST

#ifdef __cplusplus
extern "C" {
#endif
#include "unity.h"
#include "version_check.h"
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

void test_is_hash_invalid_with_valid_short_hash(void) {
    TEST_ASSERT_FALSE(is_hash_invalid("a1b2c3d"));
}

/*void test_is_hash_invalid_with_valid_dirty_hash(void) {
    TEST_ASSERT_FALSE(is_hash_invalid("a1b2c3d-dirty"));
}*/

void test_is_hash_invalid_with_null_pointer(void) {
    TEST_ASSERT_TRUE(is_hash_invalid(NULL));
}

void test_is_hash_invalid_with_empty_string(void) {
    TEST_ASSERT_TRUE(is_hash_invalid(""));
}

void test_is_hash_invalid_with_too_short_string(void) {
    TEST_ASSERT_TRUE(is_hash_invalid("a1"));
}

/*void test_is_hash_invalid_with_garbage_characters(void) {
    TEST_ASSERT_TRUE(is_hash_invalid("a1b2c3d !!!"));
    TEST_ASSERT_TRUE(is_hash_invalid("unknown")); // 'u', 'n', 'o' не входят в hex
}*/
#endif
