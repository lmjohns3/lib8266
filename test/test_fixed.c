#include <lib8266/fixed.h>
#include <unity.h>

void test_exp() {
  TEST_ASSERT_EQUAL(L8_FIXED_1, l8_fixed_exp(0));
  TEST_ASSERT_EQUAL(L8_FIXED_E, l8_fixed_exp(L8_FIXED_1));
  TEST_ASSERT_EQUAL(L8_FIXED_E * L8_FIXED_E, l8_fixed_exp(L8_TO_FIXED(2)));
}

void test_log() {
  TEST_ASSERT_EQUAL(L8_FIXED_1, l8_fixed_log(L8_FIXED_E));
  TEST_ASSERT_EQUAL(L8_TO_FIXED(2), l8_fixed_log(L8_FIXED_E * L8_FIXED_E));
}

void test_mod() {
  TEST_ASSERT_EQUAL(0, l8_fixed_mod(0, L8_FIXED_1));
  TEST_ASSERT_EQUAL(0, l8_fixed_mod(L8_FIXED_1, L8_FIXED_1));
  TEST_ASSERT_EQUAL(0, l8_fixed_mod(L8_TO_FIXED(5), L8_TO_FIXED(5)));
  TEST_ASSERT_EQUAL(0, l8_fixed_mod(L8_TO_FIXED(5.1), L8_TO_FIXED(5.1)));

  TEST_ASSERT_EQUAL(L8_FIXED_1, l8_fixed_mod(L8_TO_FIXED(5), L8_TO_FIXED(2)));
  TEST_ASSERT_EQUAL(L8_TO_FIXED(2), l8_fixed_mod(L8_TO_FIXED(5), L8_TO_FIXED(3)));

  TEST_ASSERT_EQUAL(L8_TO_FIXED(0.1), l8_fixed_mod(L8_TO_FIXED(5.1), L8_TO_FIXED(1.0)));
  TEST_ASSERT_EQUAL(L8_TO_FIXED(1.1), l8_fixed_mod(L8_TO_FIXED(5.1), L8_TO_FIXED(2.0)));
  TEST_ASSERT_EQUAL(L8_TO_FIXED(2.1), l8_fixed_mod(L8_TO_FIXED(5.1), L8_TO_FIXED(3.0)));

  TEST_ASSERT_EQUAL(L8_TO_FIXED(-0.1), l8_fixed_mod(L8_TO_FIXED(-5.1), L8_TO_FIXED(1.0)));
  TEST_ASSERT_EQUAL(L8_TO_FIXED(-1.1), l8_fixed_mod(L8_TO_FIXED(-5.1), L8_TO_FIXED(2.0)));
  TEST_ASSERT_EQUAL(L8_TO_FIXED(-2.1), l8_fixed_mod(L8_TO_FIXED(-5.1), L8_TO_FIXED(3.0)));
}

void test_pow() {
  TEST_ASSERT_EQUAL(L8_FIXED_1, l8_fixed_pow(L8_FIXED_1, 0));
  TEST_ASSERT_EQUAL(L8_FIXED_1, l8_fixed_pow(L8_FIXED_1, L8_FIXED_1));
  TEST_ASSERT_EQUAL(L8_FIXED_1, l8_fixed_pow(L8_FIXED_1, L8_TO_FIXED(2)));

  TEST_ASSERT_EQUAL(L8_TO_FIXED(2), l8_fixed_pow(L8_TO_FIXED(2), L8_FIXED_1));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_exp);
    RUN_TEST(test_log);
    RUN_TEST(test_mod);
    RUN_TEST(test_pow);
    UNITY_END();
    return 0;
}
