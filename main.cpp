#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <mutex>
#include <fstream>
#include <chrono>

using namespace std;
mutex mtx;

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
 * @param value
 * @param values
 */

void threadFunction(int value, int startingIndex, int endingIndex, vector<bool> &values) {
    endingIndex = min(max(endingIndex, (int)values.size()), (int)values.size());
    for(int i = startingIndex; i < endingIndex; i+=value) {
        values[i] = false;
    }
}

/**
 * This function is called to invoke the threads to calculate the prime numbers.
 * 
 * @param numPrimes
 * @param numThreads
 * @param values
 */


void invokeThreads(int numPrimes, int numThreads, vector<bool> &values) {
    vector<thread> threads;
    int runs = sqrt(numPrimes);

    for(int i = 2; i < runs; i++) {
        if(values[i]) {
            int start = i * i;
            int end = numPrimes;
            int range = end - start;
            int workload = ceil((double)range / (double)numThreads);

            int lastStartingValue = start;

            for(int j = 0; j < numThreads; j++) {                
                int endValue = ((lastStartingValue + workload) / i )* i;
                threads.push_back(thread(threadFunction, i, lastStartingValue, endValue, ref(values)));
                lastStartingValue = endValue;
            }

            for(auto &thread : threads) {
                thread.join();
            }
            threads.clear();
        }
    }
}

vector<int> findPrimeValues(int numPrimes, int threads) {
    vector<bool> values(numPrimes, true);
    vector<int> primes;
    values[0] = false; values[1] = false;

    invokeThreads(numPrimes, 8, values);

    for(int i = 0; i < numPrimes; i++) {
        if(values.at(i)) {
            primes.push_back(i);
        }
    }
    return primes;
}

int main(int argc, char* argv[]) {
    int numPrimes = 10000;
    int numThreads = 8;
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    ofstream myFile("file.txt");
    
    vector<int> primes = findPrimeValues(numPrimes, numThreads);
    myFile << "Count: " << primes.size() << endl;
    myFile << "Time: " << chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin).count() << " ms" << endl;
    myFile.close();
    return 0;
}