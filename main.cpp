#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <fstream>
#include <chrono>
#include "BS_thread_pool.hpp"

using namespace std;

/**
 * This program calculates the prime numbers up to a given number using the Sieve of Eratosthenes algorithm.
 * 
 * The program uses a multithreaded approach to calculate the prime numbers.
 * https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes
 */


/**
 * This function is called by each thread to calculate the prime numbers from a given number to a given number. The
 * workload is split equally by the function invokeThreads.
 * 
 * @param value the number currently being sieved
 * @param startingIndex the starting index of the subarray
 * @param endingIndex the ending index of the subarray
 * @param values the array of values to be sieved
 */

void threadFunction(int value, int startingIndex, int endingIndex, vector<bool> &values) {
    endingIndex = min(max(endingIndex, (int)values.size()), (int)values.size());
    if(startingIndex % 2 == 0){
        startingIndex -= value; // Make sure it starts with an odd number
    }
    for(int i = startingIndex; i < endingIndex; i+=value*2) { // * 2 so it iterates through odd numbers only
        values[i] = false;
    }
}

/**
 * This function is called to invoke the threads to calculate the prime numbers.
 * It works by splitting up the workload equally among the threads, performing the sieve
 * on different chunks of the array simultaneously.
 * 
 * @param numPrimes the upper bound of primes to be found
 * @param numThreads the number of threads to be used
 * @param values the array of values to be sieved
 */


chrono::steady_clock::time_point invokeThreads(int numPrimes, int numThreads, vector<bool> &values) {
    BS::thread_pool pool(numThreads);
    int runs = ceil(sqrt(numPrimes));

    for(int i = 3; i < runs; i+=2) {  // Go only through odd numbers (evens are all non-prime and code below accounts for it)
        if(values[i]) {
            int start = i * i;
            int end = numPrimes;
            int range = end - start;
            int workload = range / numThreads;

            int lastStartingValue = start;

            for(int j = 0; j < numThreads; j++) {
                // Split workload:        
                int endValue = ((lastStartingValue + workload) / i )* i;
                pool.submit_task([=, &values] {  // void to avoid return error
                    threadFunction(i, lastStartingValue, endValue, ref(values));
                });
                lastStartingValue = endValue;
            }
        }
    }
    pool.wait();
}

/**
 * This function is called to find the prime numbers up to a given number. It starts the thread function and stores
 * runtime and the sum of the prime numbers.
 * 
 * @param numPrimes upper bound of primes to be calculated
 * @param numThreads number of threads to be used
 * @return tuple<vector<int>, long long> The vector of prime numbers, the sum of the prime numbers, and the time it took to run threads.
 */

tuple<vector<int>, long long, long long> findPrimeValues(int numPrimes, int numThreads) {
    vector<bool> values(numPrimes, true);
    vector<int> primes;
    values[0] = false; values[1] = false;  // Initialize 0 and 1 as non primes.

    chrono::steady_clock::time_point begin = chrono::steady_clock::now(); // Starting time
    invokeThreads(numPrimes, numThreads, values);
    long long time = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin).count(); // Ending time
    long long sum = 0;

    // Add the prime numbers to the vector.
    primes.push_back(2); // Add 2, the only even prime number, since function only goes through odd numbers.
    for(int i = 1; i < numPrimes; i+=2) { // Goes through odd numbers (evens are not prime)
        if(values.at(i)) {
            primes.push_back(i);
            sum += i;
        }
    }

    return {primes, time, sum};
}

int main(int argc, char* argv[]) {
    int numPrimes = 100000000;
    int numThreads = 8;
    
    ofstream myFile("file.txt");
    auto primes = findPrimeValues(numPrimes, numThreads); // tuple<prime vector, runtime, sum>
    myFile << "Count: " << get<0>(primes).size() << endl;
    myFile << "Time: " << get<1>(primes) << " ms" << endl;
    myFile << "Sum of primes: " << get<2>(primes) << endl;
    myFile << "Top 10 maximum primes: ";
    for(int i = get<0>(primes).size() - 10; i < get<0>(primes).size(); i++) {
        myFile << get<0>(primes)[i] << " ";
    }
    myFile.close();
    return 0;
}