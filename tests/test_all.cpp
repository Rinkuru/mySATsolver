#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <string>
#include "Solver.h"

TEST(FileTest, CompareWithExpectedOutput) {
    for (int i = 1; i <= 100; ++i) {
        std::ostringstream oss;
        oss<<i;
        std::string input_file = "UF150/uf150-0" + oss.str() + ".cnf";
        std::cerr << i << " ";
        if (i % 10 == 0) std::cerr << std::endl;
        
        SAT a(input_file);
        EXPECT_EQ(a.get_result(), "SAT") << "Выход не совпадает с ожидаемым!";
   }
}

/*
TEST(FileTest, CompareWithExpectedOutput) {
    std::string input_file = "hanoi4.cnf";
    SAT a(input_file);
    EXPECT_EQ(a.get_result(), "SAT") << "Выход не совпадает с ожидаемым!";
}*/

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
