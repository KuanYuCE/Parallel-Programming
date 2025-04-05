#include <iostream>
#include <ctime>
#include <getopt.h>
#include <random>
#include <climits>
#include <string>
#include <cstdlib>
#include <chrono>
#include <pthread.h>
#include <immintrin.h>
#include "pcg_random.hpp"

using namespace std;


size_t BATCH_SIZE  = 8;
struct MonteArgs {
    unsigned int id;
    long long chunk;
    long long count;
};


void* monte_carlo(void* arg) {
    long long number_in_circle = 0;
    MonteArgs* args = (MonteArgs*)arg;
    unsigned int *seed;
    unsigned int *seed2;
    unsigned int s = args->id;
    unsigned int s2 = args->id+1;
    seed = &s;

    pcg_extras::seed_seq_from<std::random_device> seed_source;
    pcg32 rng(s);
    size_t iteration_per_thread = (args->chunk / BATCH_SIZE) * BATCH_SIZE;
    for (long long i = 0; i < args->chunk; i += BATCH_SIZE) {
        alignas(32) float x_vals[BATCH_SIZE];
        alignas(32) float y_vals[BATCH_SIZE];

        constexpr float SCALE = 1.0f / 4294967295.0f;  // 2^32 - 1

        for (size_t j = 0; j < BATCH_SIZE; ++j) {
            x_vals[j] = rng() * SCALE;
            y_vals[j] = rng() * SCALE;
        }


        __m256 x = _mm256_load_ps(x_vals);
        __m256 y = _mm256_load_ps(y_vals);

        __m256 x2 = _mm256_mul_ps(x, x);
        __m256 y2 = _mm256_mul_ps(y, y);
        __m256 sum = _mm256_add_ps(x2, y2);

        __m256 one = _mm256_set1_ps(1.0f);
        __m256 cmp = _mm256_cmp_ps(sum, one, _CMP_LE_OS);

        int mask = _mm256_movemask_ps(cmp);
        number_in_circle += __builtin_popcount(mask);
    }
    args->count = number_in_circle;

    return nullptr;
}

int main(int argc, char* argv[]) {
    unsigned int thread_count = 0;
    long long int tosses = 1e8;
    bool has_thread_count = false;
    bool has_tosses = false;
    bool verbose = false;
    int opt;
    int option_index = 0;
    void usage(const char *prog_name);
    
    const struct option long_options[] = {
        {"threads_count", required_argument, nullptr, 'c'},
        {"toss", required_argument, nullptr, 't'},
        {"verbose", no_argument, nullptr, 'v'},
        {"help", no_argument, nullptr, 'h'},
        {0, 0, 0, 0}
    };
    while ((opt = getopt_long(argc, argv, "c:t:vh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'c':
                try {
                    thread_count = stoi(optarg);
                    has_thread_count = true;
                } catch (...) {
                    cerr << "Invalid number for -c\n";
                    return 1;
                }
                break;
            case 't':
                try {
                    tosses = stoll(optarg);
                    has_tosses = true;
                } catch (...) {
                    cerr << "Invalid number for -t\n";
                    return 1;
                }
                break;
            case 'v':
                verbose = true;
                break;
            case 'h':
            default:
                usage(argv[0]);
                return 0;
        }
    }

    int remaining = argc - optind;
    if (remaining >= 1 && !has_thread_count) thread_count = stoi(argv[optind]);
    if (remaining >= 2 && !has_tosses) tosses = stoll(argv[optind + 1]);

    pthread_t threads[thread_count];
    MonteArgs args[thread_count];
    int chunk = tosses / thread_count;
    int remainder = tosses % thread_count;
    
    // create thread
    for (unsigned int i = 0; i < thread_count; ++i) {
        if (i == thread_count-1) {
            chunk += remainder;
        }
        args[i] = {i, chunk, 0};
        pthread_create(&threads[i], nullptr, monte_carlo, &args[i]);
    }

    // wait for thread to completed
    for (unsigned int i = 0; i < thread_count; ++i) {
        pthread_join(threads[i], nullptr);
    }
    long long new_sum = 0;
    for (size_t i = 0; i < thread_count; ++i) {
        new_sum += args[i].count;
    }
    double pre_divide = (double) 1 / tosses;
    printf("%lf\n", 4.0 * new_sum * pre_divide) ;
    return 0;
}

void usage(const char *progname)
{
  printf("Usage: %s [options]\n", progname);
  printf("Program Options:\n");
  printf("  -c  --threads_count <N>  the number of threads (Default = 1)\n");
  printf("  -t  --toss <N>           the number of tosses (at least 1e8)\n");
  printf("  -h  --help               print usage\n");
}
