#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <fstream>
#include <chrono>
#include "BS_thread_pool.hpp"
#include <future>

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


void sieveValue(vector<bool> &primes, int i, int threadNum){

    // This function will iterate through the wheeled values only using the difference between the numbers to traverse through.
    // This avoids checking multiples of 2, 3, and 5 altogether.

    vector<int> offsets = {4, 2, 4,  2,  4,  6,  2,  6};
    int count = 0;
    int wheelValue = 7;
    int value = (i*2)+1;
    int j = value * value;
    while (j < sqrt(MAX_PRIME)) {
        primes[(j-1)/2] = false;
        wheelValue += offsets[count % 8];
        j = (value * wheelValue);
        count++;
    }
}

vector<int> initialSieve(vector<bool> &primes){

    // Sieve up to the square root of the max prime to find the primes, then use those primes to sieve the rest of the numbers.
    // This function will iterate through the wheeled values only using the difference between the numbers to traverse through.
    // This avoids checking multiples of 2, 3, and 5 altogether.

    // It adds prime numbers to the int vector, then removes all multiples of that prime number from the bool vector.

    vector<int> offsets = {4, 2, 4,  2,  4,  6,  2,  6};
    vector<int> intPrimeVector = {2, 3, 5};
    int index = -1;
    int limit = sqrt(MAX_PRIME);
    for(int i = 7; i < limit; i+=offsets[index % 8]){
        if(primes[i/2]){
            intPrimeVector.push_back(i);
            sieveValue(primes, i/2, 0);
        }
       index++;
    }
    return intPrimeVector;
}

void chunkSieve(vector<bool> &wheel, vector<int> &primes, int threadID){

    // This function will iterate through the wheeled values only using the difference between the numbers to traverse through.
    // It accounts for split up chunks of the wheel, so each thread will only calculate a portion of the wheel.

    vector<int> offsets = {4, 2, 4, 2, 4, 6, 2, 6};
    unordered_map<int, int> wheelLookup = {{1, 0}, {7, 1}, {11, 2}, {13, 3}, {17, 4}, {19, 5}, {23, 6}, {29, 7}}; // to get the index of the offset
    int end = min((threadID + 1) * (MAX_PRIME / MAX_THREADS), MAX_PRIME);
    
    for(auto it = primes.begin() + 3; it != primes.end(); ++it){  // Skip checking 2, 3, and 5
        int prime = *it;
        int start = ((((threadID) * (MAX_PRIME / MAX_THREADS) + (prime - 1))/prime)*prime);  // get first divisible by prime number
        if (start % 2 == 0) { start += prime; }  // make sure it is odd
        if (threadID == 0) { start = prime * prime; }  // start at prime squared if thread 0

        while (wheelLookup.find(start % 30) == wheelLookup.end()){
            start += prime * 2;
        }
        if(start > end){ break; }  // skip if start is greater than end
        
        int wheelValue = start / prime;
        int index = wheelLookup[wheelValue % 30] + 7;

        while (wheelValue * prime < end){
            wheel[(wheelValue * prime - (threadID * (MAX_PRIME / MAX_THREADS))) / 2] = false;
            wheelValue += offsets[index % 8];
            index++;
        }
    }
}

void sieveVector(vector<vector<bool>> &wheel){

    // We will sieve up to the square root of MAX_PRIME, so we can get all prime numbers up to that number and use those to sieve
    // This works since all non-prime numbers have a prime factor less than or equal to the square root of the number.

    vector<bool> wheelSubset = {wheel[0].begin(), wheel[0].begin() + (sqrt(MAX_PRIME) / 2) - 1};
    vector<int> primes = initialSieve(wheelSubset);
    
    for(int i = 0; i < MAX_THREADS; i++){
        THREAD_POOL.detach_task([=, &wheel, &primes] () {
            chunkSieve(ref(wheel[i]), ref(primes), i);
        });
    }
    THREAD_POOL.wait();
}

vector<bool> individualWheelValue(int startValue, int endValue){

    // This function will calculate the wheel values for a specific chunk of the wheel.
    // It will only calculate the values that are part of the wheel, skipping multiples of 2, 3, and 5.

    vector<int> offsets = {4, 2, 4,  2,  4,  6,  2,  6};
    unordered_map<int, int> wheelLookup = {{1, 0}, {7, 1}, {11, 2}, {13, 3}, {17, 4}, {19, 5}, {23, 6}, {29, 7}}; // to get the index of the offset
    vector<bool> wheel((endValue - startValue)/2 + 1, false);

    int value = startValue;

    // Finds next value that will be part of the wheel. Useful if you start, for example, at 1250, which is not part of the wheel.
    // This is needed since the calculations are split between 8 chunks.

    while (wheelLookup.find(value % 30) == wheelLookup.end()) { value++; }  
    int index = wheelLookup[value % 30] + 7;

    while(value < endValue){  // calculate the wheel values
        wheel[(value - startValue) / 2] = true;
        value += offsets[index % 8];
        index++;
    }
    return wheel;
}

void wheelFactorization(vector<vector<bool>> &wheel){
    vector<future<vector<bool>>> futures;
    futures.reserve(MAX_THREADS);
    wheel.reserve(MAX_THREADS);

    // Split the vector into 8 vectors, each for a thread to run the wheel, following the pattern 4 2 4 2 4 6 2 6.
    // This pattern is the difference between 1, 7, 11, 13, 17, 19, 23, 29, which are the first 8 values of the wheel.
    // These values are obtained by removing all multiples of 2, 3, and 5 from the numbers between 1 and 30.
    // We use 30 since the wheel removes multiples of 2, 3, and 5. 2 * 3 * 5 = 30. Our wheel is mod 30.

    for(int i = 0; i < MAX_THREADS; i++){
        int start = i * (MAX_PRIME / MAX_THREADS);
        int end = min((i + 1) * (MAX_PRIME / MAX_THREADS), MAX_PRIME);
        futures.push_back(THREAD_POOL.submit_task([=] () {
            return individualWheelValue(start, end);
        }));
    }

    // This function adds the wheel values to the wheel vector. It uses futures which makes it wait for all the threads to finish.
    for(auto &future : futures){
        wheel.push_back(future.get());
    }
    
    // Then, add primes 3 and 5 to the wheel. No need for 2, since we only store even numbers.
    wheel[0][0] = false; // 1 is not prime.
    wheel[0][1] = true;  // 3 is prime.
    wheel[0][2] = true;  // 5 is prime.
}

vector<long long> boolToIntVector(vector<bool> &primes, int threadID){

    // This function converts the bool vector to an int vector, then adds the sum of the primes to the end of the vector.
    // It is optimized for multithreading, so each thread will only calculate a portion of the wheel.
    // It will only calculate the values that are part of the wheel, skipping multiples of 2, 3, and 5.

    vector<long long> intPrimes;
    vector<int> offsets = {4, 2, 4, 2, 4, 6, 2, 6};
    unordered_map<int, int> wheelLookup = {{1, 0}, {7, 1}, {11, 2}, {13, 3}, {17, 4}, {19, 5}, {23, 6}, {29, 7}};
    int firstPrimeInChunk = 0;
    long long sum = 0;

    if(threadID == 0){
        intPrimes = {2, 3, 5}; // 2, 3, 5 are prime, but will not be calculated with the wheel so they have to be manually added.
        sum = 2 + 3 + 5;  // also add the sum of the first 3 primes.
        firstPrimeInChunk = 7;  // start at 7, since 2, 3, 5 are already added.
    } else { // find the first prime in this chunk.
        while(!primes[firstPrimeInChunk]){ firstPrimeInChunk++; }  // find the index of first prime in the chunk
        firstPrimeInChunk = ((firstPrimeInChunk * 2) + 1) + (threadID * (MAX_PRIME / MAX_THREADS));  // now, convert to value
    }
    
    int index = wheelLookup[firstPrimeInChunk % 30] + 7;
    int end = min((threadID + 1) * (MAX_PRIME / MAX_THREADS), MAX_PRIME);

    while (firstPrimeInChunk < end){
        if(primes[(firstPrimeInChunk - (threadID * (MAX_PRIME / MAX_THREADS))) / 2]){
            sum += firstPrimeInChunk;
            intPrimes.push_back(firstPrimeInChunk);
        }
        firstPrimeInChunk += offsets[index % 8];
        index++;
    }
    intPrimes.push_back(sum);
    return intPrimes;
}

int main(int argc, char** argv){
    vector<vector<bool>> wheel;
    auto begin = chrono::steady_clock::now(); // Starting time
    wheelFactorization(wheel);
    sieveVector(wheel);
    vector<future<vector<long long>>> primes;
    vector<vector<long long>> primeVector;
    primes.reserve(MAX_THREADS);
    primeVector.reserve(MAX_THREADS);
    for(int i = 0; i < MAX_THREADS; i++){
        primes.push_back(THREAD_POOL.submit_task([=, &wheel] () {
            return boolToIntVector(wheel[i], i);
        }));
    }
    for(auto &prime : primes){
        primeVector.push_back(prime.get());
    }
    auto time = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin).count(); // Ending time

    long long sum = 0;
    int count = 0;
    for(int i = 0; i < primeVector.size(); i++){
        sum += primeVector[i].back();
        count += primeVector[i].size() - 1;
    }
    ofstream file("primes.txt");
    file << "Run time: " << time << " ms" << endl;
    file << "Total primes: " << count << endl;
    file << "Sum of primes: " << sum << endl;
    file << "Top ten maximum primes: " << endl;

    for(auto it = primeVector[MAX_THREADS-1].end() - 11; it != primeVector[MAX_THREADS-1].end() - 1; ++it){  // Skip checking 2, 3, and 5
        file << *it << " ";
    }
    file.close();
    return 0;
}