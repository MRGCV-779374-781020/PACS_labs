#include <iomanip>
#include <iostream>
#include <limits>
#include <chrono>
#include <vector>

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
    if (argc < 2) {
        std::cerr << "Invalid syntax: pi_taylor <steps>+ [--repeat <num_repeats>]" << std::endl;
        exit(1);
    }
    int num_repeats = 1;
    if (std::string(argv[argc - 2]) == "--repeat") {
        num_repeats = std::stoi(argv[argc - 1]);    
        argc -= 2;
    }
    // measure time with std::chrono
    std::cout << "steps, pi value, step time (ns), total time (ns)" << std::endl;
    std::vector<std::chrono::nanoseconds> exec_times;
    for (int i = 1; i < argc; i++) {
        size_t steps = std::stoll(argv[i]);
        auto pi = pi_taylor(steps);
        auto start = std::chrono::steady_clock::now();
        for (int j = 0; j < num_repeats; j++) {
            pi = pi_taylor(steps);
        }
        auto stop = std::chrono::steady_clock::now();

        std::cout << steps << ", "
            << std::setprecision(std::numeric_limits<my_float>::digits10 + 1)
            << pi
            << ", "
            << std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count() / (num_repeats * steps)
            << ", "
            << std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count() / num_repeats
            << std::endl;
    }
}
