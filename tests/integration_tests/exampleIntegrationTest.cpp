#include <gtest/gtest.h>
#include "TestClass.h"
#include "TestClass2.h"

TEST(exampleIntegrationTest, EmptyTest) {
TestClass tc;
TestClass2 tc2;
ASSERT_EQ(tc.getBool(true), tc2.getBool(true));
}