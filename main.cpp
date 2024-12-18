#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <fstream>
#include <chrono>
#include "BS_thread_pool.hpp"

using namespace std;

const int MAX_THREADS = 8;
const int MAX_PRIME = 100;

/**
 * This program calculates the prime numbers up to a given number using the Sieve of Eratosthenes algorithm.
 * 
 * The program uses a multithreaded approach to calculate the prime numbers.
 * https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes
 */

void sieveValue(vector<bool> &primes, int i){
    int value = (i*2)+3;
    for(int j = value * value; j < MAX_PRIME; j += (value * 2)){ // Skip even numbers
        cout << j << ", Index to be removed " << (j-3)/2 << endl;
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


int main(int argc, char** argv){
    vector<bool> primes((MAX_PRIME/2 - 1), true);  // (3, end] inclusive.
    sieveVector(primes);

    for(int i = 0; i < primes.size(); i++){
        if(primes[i]){
            cout << i*2+3 << " ";
        }
    }
}