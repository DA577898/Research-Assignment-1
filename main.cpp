#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <fstream>
#include <chrono>
#include "BS_thread_pool.hpp"

using namespace std;

const int MAX_THREADS = 8;
const int MAX_PRIME = 100000000;
BS::thread_pool THREAD_POOL(MAX_THREADS);


/**
 * This program calculates the prime numbers up to a given number using the Sieve of Eratosthenes algorithm.
 * 
 * The program uses a multithreaded approach to calculate the prime numbers.
 * https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes
 */

void sieveValue(vector<bool> &primes, int i){
    int value = (i*2)+3;
    for(int j = value * value; j < MAX_PRIME; j += (value * 2)){ // Skip even numbers
        primes[(j-3)/2] = false;
    }
}

void sieveVector(vector<bool> &primes){
    int limit = sqrt(MAX_PRIME) / 2;  // Avoid recalculating every run
    for(int i = 0; i < limit; i++){
        if(primes[i]){
            sieveValue(primes, i);
        }
    }
}

void individualWheelValue(vector<bool> &primes, int value){
    for(int j = value + 30; j < MAX_PRIME; j += 30){ // Every 30 mark the next possible prime following the wheel pattern since it's a mod 30 wheel.
        primes[(j-3)/2] = true;
    }
}

void wheelFactorization(vector<bool> &primes){
    // First, set primes from 0 to thirty to true.
    vector<int> primesToThirty = {3, 5, 7, 11, 13, 17, 19, 23, 29};  // Set first primes excluding 2 since it will be added at the final calculation. 
    for(auto i : primesToThirty){
        primes[(i-3)/2] = true;
    }

    // Then using the wheel create a list of potential primes (up to sqrt of max prime since it will be iterated until that number).
    // The numbers are obtained by removing all multiples of 2, 3, 5, leaving us with 8 numbers ( = to thread count).
    // Since 2*3*5 = 30, the values will repeat every 30 numbers to form the pattern. (mod 30)
    vector<int> wheel = {1, 7, 11, 13, 17, 19, 23, 29};  // Size of wheel is 8, since we have 8 threads.

    int chunks = sqrt(MAX_PRIME) / 30;  // Number of chunks to divide the wheel into.

    for(int i = 0; i < MAX_THREADS; i++){
        THREAD_POOL.detach_task([=, &primes] {
            individualWheelValue(ref(primes), wheel[i]);
        });
        THREAD_POOL.wait();
    }
}

int main(int argc, char** argv){
    vector<bool> primes((MAX_PRIME/2 - 1), false);  // (3, end] inclusive.
    wheelFactorization(primes);
    sieveVector(primes);

    ofstream file("file.txt");
    for(int i = 0; i < primes.size(); i++){
        if(primes[i]){
            file << (i*2)+3 << ", ";
        }
    }
    file << endl << "Total primes: " << count(primes.begin(), primes.end(), true) + 1;  // Add 2 to the count.
}