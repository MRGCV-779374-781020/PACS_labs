#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <chrono>
#include <cmath>

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
    my_float y, t;

    std::vector<my_float>::iterator it;

    for (it = input.begin(); it != input.end(); it++) {
        y = *it - c;
        t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }

    return sum;
}

struct args{
    std::vector<size_t> num_threads;
    size_t num_steps;
    size_t num_repeats;
};

args
usage(int argc, const char *argv[]) {
    // read the number of steps from the command line
    if (argc < 3) {
        std::cerr << "Invalid syntax: pi_taylor <steps> <threads>+ [--repeat <num_repeats>]" << std::endl;
        exit(1);
    }

    args ret_args;

    ret_args.num_steps = std::stoll(argv[1]);
    
    int num_opts = argc - 2;

    ret_args.num_repeats = 1;
    if (std::string(argv[argc - 2]) == "--repeat") {
        ret_args.num_repeats = std::stoll(argv[argc - 1]);    
        num_opts -= 2;
    }    
    
    for (int i = 0; i < num_opts; i++) {
        size_t threads = std::stoll(argv[i+2]);
        if (threads > ret_args.num_steps) {
            std::cerr << "The number of steps should be larger than the number of threads" << std::endl;
            exit(1);
        }    
        ret_args.num_threads.push_back(threads);
    }

    return ret_args;
}

struct results {
    size_t num_threads;
    size_t num_steps;
    size_t num_repeats;
    my_float pi;
    my_float time_cv;
    my_float time_mean;
    my_float time_std_dev;

    friend std::ostream& operator<<(std::ostream& os, const results& res) {
        os << res.num_threads << ", "
            << res.num_steps << ", "
            << std::setprecision(std::numeric_limits<my_float>::digits10 + 1)
            << res.pi
            << ", "
            << res.time_mean / (res.num_repeats * res.num_steps)
            << ", "
            << res.time_mean
            << ", "
            << res.time_cv;
        return os;
    }
};


int main(int argc, const char *argv[]) {


    auto ret_args = usage(argc, argv);
    auto steps = ret_args.num_steps;
    auto num_repeats = ret_args.num_repeats;
    std::vector<results> threading_results;

    std::cout << "threads, steps, pi value, step time (ns), total time (ns), coeficient of variation (time)" << std::endl;
    for (std::vector<long unsigned int>::size_type i = 0; i < ret_args.num_threads.size(); i++) {
        std::vector<std::chrono::nanoseconds> exec_times;
        my_float pi = 0.0;
        auto threads = ret_args.num_threads[i];
        for (long unsigned int j = 0; j < num_repeats; j++) {
            auto start = std::chrono::steady_clock::now();

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

            auto stop = std::chrono::steady_clock::now();
            exec_times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start));
        }
        
        results res;
        res.num_threads = threads;
        res.num_steps = steps;
        res.num_repeats = num_repeats;
        res.pi = pi;
        res.time_mean = 0.0;
        for (auto &t : exec_times) {
            res.time_mean += t.count();
        }
        res.time_mean /= exec_times.size();
        
        my_float var = 0.0;
        for (auto &t : exec_times) {
            var += (t.count() - res.time_mean) * (t.count() - res.time_mean);
        }
        var /= exec_times.size();

        res.time_std_dev = sqrt(var);

        res.time_cv = res.time_std_dev / res.time_mean;

        std::cout << res << std::endl;

        exec_times.clear();

        threading_results.push_back(res);

    }

    // calculate coeficient of variation for pi
    my_float pi_mean = 0.0;
    for (auto &res : threading_results) {
        pi_mean += res.pi;
    }
    pi_mean /= threading_results.size();

    my_float pi_var = 0.0;
    for (auto &res : threading_results) {
        pi_var += (res.pi - pi_mean) * (res.pi - pi_mean);
    }
    pi_var /= threading_results.size();

    my_float pi_std_dev = sqrt(pi_var);

    my_float pi_cv = pi_std_dev / pi_mean;

    std::cout << "Coeficient of variation for pi: " << pi_cv << std::endl;
}

