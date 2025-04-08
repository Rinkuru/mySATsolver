#include "Solver.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "not found file\n";
        return 0;
    }
    std::string str = argv[1];
    SAT a(str);
    std::cout << a.get_result() << std::endl;
    return 0;
}
