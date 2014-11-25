#include <iostream>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <random>
#include <chrono>
#include <sparsehash/sparsetable>

using google::sparsetable;
using namespace std;

// Benchmark tool to test 1/8 filling a sparsetable with 1<<32 slots
// and reading and writing it to disk. Delete the test file to start again
// Output from first and second run below:

/*
$ g++-4.9 -O3 --std=c++11 hash.cc -I. -o hash && time ./hash test
Added 536870911 random keys to a sparsetable of width 4294967295 in 203 seconds
Iterated 504665711 stored keys in a sparsetable of width 4294967295 in 0 seconds
Wrote 504665711 stored keys to a sparsetable of width 4294967295 in 69 seconds

real    5m23.025s
user    5m11.155s
sys 0m11.749s
$ g++-4.9 -O3 --std=c++11 hash.cc -I. -o hash && time ./hash test
Read 504665711 stored keys from a sparsetable of width 4294967295 in 53 seconds
Iterated 504665711 stored keys in a sparsetable of width 4294967295 in 0 seconds
Wrote 504665711 stored keys to a sparsetable of width 4294967295 in 55 seconds

real    1m55.734s
user    1m45.852s
sys 0m9.840s
*/

const auto WIDTH = numeric_limits<uint32_t>::max();
const auto N = WIDTH / 8;

// Pass the filename as the argument
int main(int argc, char* argv[])
{
    if (argc != 2)
        exit(1);

    sparsetable<uint64_t> table;
    auto file = fopen(argv[1], "r");
    if (!file)
    {
        // Fill the file
        auto start = chrono::high_resolution_clock::now();

        table.resize(WIDTH);
        mt19937 gen;
        gen.seed(0);
        uniform_int_distribution<uint32_t> dis(0, WIDTH - 1);
        for (size_t i = 0; i < (N); i++)
        {
            table[dis(gen)] = i;
        }

        auto elapsed = chrono::high_resolution_clock::now() - start;
        cout << "Added " << N << " random keys to a sparsetable of width "
             << WIDTH << " in "
             << chrono::duration_cast<chrono::seconds>(elapsed).count()
             << " seconds" << endl;
    }
    else
    {
        // Read the file
        auto start = chrono::high_resolution_clock::now();

        if (!table.read_metadata(file) || !table.read_nopointer_data(file) ||
            fclose(file))
            exit(1);

        auto elapsed = chrono::high_resolution_clock::now() - start;
        cout << "Read " << table.num_nonempty()
             << " stored keys from a sparsetable of width " << WIDTH << " in "
             << chrono::duration_cast<chrono::seconds>(elapsed).count()
             << " seconds" << endl;
    }
    // Iterate the file
    auto start = chrono::high_resolution_clock::now();

    size_t i = 0;
    for (auto it = table.nonempty_begin(); it != table.nonempty_end(); ++it)
        i++;

    auto elapsed = chrono::high_resolution_clock::now() - start;
    cout << "Iterated " << i << " stored keys in a sparsetable of width "
         << WIDTH << " in "
         << chrono::duration_cast<chrono::seconds>(elapsed).count()
         << " seconds" << endl;

    // Write the file
    start = chrono::high_resolution_clock::now();
    file = fopen(argv[1], "w");
    if (!table.write_metadata(file) || !table.write_nopointer_data(file) ||
        fclose(file))
        exit(1);

    elapsed = chrono::high_resolution_clock::now() - start;
    cout << "Wrote " << table.num_nonempty()
         << " stored keys to a sparsetable of width " << WIDTH << " in "
         << chrono::duration_cast<chrono::seconds>(elapsed).count()
         << " seconds" << endl;
    return 0;
}