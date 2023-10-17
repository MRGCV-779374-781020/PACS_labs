#include <iomanip>
#include <iostream>
#include <limits>

// Allow to change the floating point type
using my_float = long double;

my_float pi_taylor(size_t steps) {
    my_float pi = 0.0;
    my_float sign = 1.0;
    for (size_t i = 0; i < steps; i++) {
        pi += sign / (2 * i + 1);
        sign *= -1;
    }
    return 4.0 * pi;
}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        std::cerr << "Invalid syntax: pi_taylor <steps>" << std::endl;
        exit(1);
    }

    size_t steps = std::stoll(argv[1]);
    auto pi = pi_taylor(steps);

    std::cout << "For " << steps << ", pi value: "
        << std::setprecision(std::numeric_limits<my_float>::digits10 + 1)
        << pi << std::endl;
}
