#include <lib8266/fixed.h>
#include <unity.h>

void test_exp() {
  TEST_ASSERT_EQUAL(AHOY_FIXED_1, ahoy_fixed_exp(0));
  TEST_ASSERT_EQUAL(AHOY_FIXED_E, ahoy_fixed_exp(AHOY_FIXED_1));
  TEST_ASSERT_EQUAL(AHOY_FIXED_E * AHOY_FIXED_E, ahoy_fixed_exp(AHOY_TO_FIXED(2)));
}

void test_log() {
  TEST_ASSERT_EQUAL(AHOY_FIXED_1, ahoy_fixed_log(AHOY_FIXED_E));
  TEST_ASSERT_EQUAL(AHOY_TO_FIXED(2), ahoy_fixed_log(AHOY_FIXED_E * AHOY_FIXED_E));
}

void test_mod() {
  TEST_ASSERT_EQUAL(0, ahoy_fixed_mod(0, AHOY_FIXED_1));
  TEST_ASSERT_EQUAL(0, ahoy_fixed_mod(AHOY_FIXED_1, AHOY_FIXED_1));
  TEST_ASSERT_EQUAL(0, ahoy_fixed_mod(AHOY_TO_FIXED(5), AHOY_TO_FIXED(5)));
  TEST_ASSERT_EQUAL(0, ahoy_fixed_mod(AHOY_TO_FIXED(5.1), AHOY_TO_FIXED(5.1)));

  TEST_ASSERT_EQUAL(AHOY_FIXED_1, ahoy_fixed_mod(AHOY_TO_FIXED(5), AHOY_TO_FIXED(2)));
  TEST_ASSERT_EQUAL(AHOY_TO_FIXED(2), ahoy_fixed_mod(AHOY_TO_FIXED(5), AHOY_TO_FIXED(3)));

  TEST_ASSERT_EQUAL(AHOY_TO_FIXED(0.1), ahoy_fixed_mod(AHOY_TO_FIXED(5.1), AHOY_TO_FIXED(1.0)));
  TEST_ASSERT_EQUAL(AHOY_TO_FIXED(1.1), ahoy_fixed_mod(AHOY_TO_FIXED(5.1), AHOY_TO_FIXED(2.0)));
  TEST_ASSERT_EQUAL(AHOY_TO_FIXED(2.1), ahoy_fixed_mod(AHOY_TO_FIXED(5.1), AHOY_TO_FIXED(3.0)));

  TEST_ASSERT_EQUAL(AHOY_TO_FIXED(-0.1), ahoy_fixed_mod(AHOY_TO_FIXED(-5.1), AHOY_TO_FIXED(1.0)));
  TEST_ASSERT_EQUAL(AHOY_TO_FIXED(-1.1), ahoy_fixed_mod(AHOY_TO_FIXED(-5.1), AHOY_TO_FIXED(2.0)));
  TEST_ASSERT_EQUAL(AHOY_TO_FIXED(-2.1), ahoy_fixed_mod(AHOY_TO_FIXED(-5.1), AHOY_TO_FIXED(3.0)));
}

void test_pow() {
  TEST_ASSERT_EQUAL(AHOY_FIXED_1, ahoy_fixed_pow(AHOY_FIXED_1, 0));
  TEST_ASSERT_EQUAL(AHOY_FIXED_1, ahoy_fixed_pow(AHOY_FIXED_1, AHOY_FIXED_1));
  TEST_ASSERT_EQUAL(AHOY_FIXED_1, ahoy_fixed_pow(AHOY_FIXED_1, AHOY_TO_FIXED(2)));

  TEST_ASSERT_EQUAL(AHOY_TO_FIXED(2), ahoy_fixed_pow(AHOY_TO_FIXED(2), AHOY_FIXED_1));
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
