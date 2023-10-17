#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using my_float = float;

void
pi_taylor_chunk(std::vector<my_float> &output,
        size_t thread_id, size_t start_step, size_t stop_step) {
    my_float sum = 0.0;
    my_float c = 0.0;
    my_float aux, y, t;
    int sign = start_step & 0x1 ? -1 : 1;

    for (size_t i = start_step; i < stop_step; i++) {
        aux = sign / static_cast<my_float>(2 * i + 1);
        sign *= -1;
        y = aux - c;
        t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }

    output[thread_id] = 4*sum;
}

my_float
kahan_sum(std::vector<my_float> input)
{
    my_float sum = 0.0;
    my_float c = 0.0;
    my_float aux, y, t;

    std::vector<my_float>::iterator it;

    for (it = input.begin(); it != input.end(); it++) {
        y = *it - c;
        t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }

    return sum;
}

std::pair<size_t, size_t>
usage(int argc, const char *argv[]) {
    // read the number of steps from the command line
    if (argc != 3) {
        std::cerr << "Invalid syntax: pi_taylor <steps> <threads>" << std::endl;
        exit(1);
    }

    size_t steps = std::stoll(argv[1]);
    size_t threads = std::stoll(argv[2]);

    if (steps < threads ){
        std::cerr << "The number of steps should be larger than the number of threads" << std::endl;
        exit(1);

    }
    return std::make_pair(steps, threads);
}

int main(int argc, const char *argv[]) {


    auto ret_pair = usage(argc, argv);
    auto steps = ret_pair.first;
    auto threads = ret_pair.second;

    my_float pi;

    // please complete missing parts
    // kahan summation algorithm split the sum in {thread} parts
    // and then sum the partial sums
    std::vector<std::thread> thread_pool;

    std::vector<my_float> partial_results(threads);

    for (size_t i = 0; i < threads; i++) {
        size_t start_step = i*(steps/threads);
        size_t stop_step = (i+1)*(steps/threads);
        thread_pool.emplace_back(pi_taylor_chunk, std::ref(partial_results), i, start_step, stop_step);
    }

    for (auto &t : thread_pool) {
        t.join();
    }

    pi = kahan_sum(partial_results);

    std::cout << "For " << steps << ", pi value: "
        << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
        << pi << std::endl;
}

