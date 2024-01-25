#include "gtest/gtest.h"
#include "include/buried.h"

TEST(BuriedBasicTest, spdlogtest) {
	BuriedCreate("C:/Users/asus/Downloads");
}


int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}