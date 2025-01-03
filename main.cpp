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
    vector<int> offsets = {4, 2, 4,  2,  4,  6,  2,  6};
    
    //unordered_map<int, int> wheelLookup = {{1, 0}, {7, 1}, {11, 2}, {13, 3}, {17, 4}, {19, 5}, {23, 6}, {29, 7}}; // to get the index of the offset
    //while (wheelLookup.count(value % 30) == 0) { value++; }  // find next value that will be part of the wheel

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

void initialSieve(vector<bool> &primes){

    // Sieve up to the square root of the max prime to find the primes, then use those primes to sieve the rest of the numbers.
    // This function will iterate through the wheeled values only using the difference between the numbers to traverse through.
    // This avoids checking multiples of 2, 3, and 5 altogether.

    vector<int> offsets = {4, 2, 4,  2,  4,  6,  2,  6};
    int index = -1;
    int limit = 100;
    for(int i = 7; i < limit; i+=offsets[index % 8]){
        if(primes[i/2]){
            sieveValue(primes, i/2, 0);
        }
       index++;
    }
}

void sieveVector(vector<vector<bool>> &wheel){
    // We will sieve up to the square root of MAX_PRIME, so we can get all prime numbers.
    vector<bool> primes = {wheel[0].begin(), wheel[0].begin() + (sqrt(MAX_PRIME) / 2) - 1};
    initialSieve(primes);


/*     int count = 1;  // 2 is prime.
    for (int i = 0; i < wheel.size(); i++) {
        for (int j = 0; j < wheel[i].size(); j++) {
            if (wheel[i][j]) {
                cout << "Thread " << i << " value: " << ((i * (MAX_PRIME / MAX_THREADS)) + (2 * j)) + 1 << " index: " << j << endl;
                count++;
            }
        }
    }
    cout << endl << count << endl; */

}

vector<bool> individualWheelValue(int startValue, int endValue){
    vector<int> offsets = {4, 2, 4,  2,  4,  6,  2,  6};
    unordered_map<int, int> wheelLookup = {{1, 0}, {7, 1}, {11, 2}, {13, 3}, {17, 4}, {19, 5}, {23, 6}, {29, 7}}; // to get the index of the offset
    vector<bool> wheel((endValue - startValue)/2 + 1, false);

    int value = startValue;

    // Finds next value that will be part of the wheel. Useful if you start, for example, at 1250, which is not part of the wheel.
    // This is needed since the calculations are split between 8 chunks.

    while (wheelLookup.count(value % 30) == 0) { value++; }  
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

// For testing purposes, test multiple times and get the average run speed.

void run(vector<long long> &runTimes){
    vector<vector<bool>> wheel;
    auto begin = chrono::steady_clock::now(); // Starting time

    wheelFactorization(wheel);
    sieveVector(wheel);

    auto time = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin).count(); // Ending time
    runTimes.push_back(time);    
}

int main(int argc, char** argv){
    vector<long long> runTimes;
    for(int i = 0; i < 10; i++){run(runTimes);}
    long long total = 0;
    for (int time : runTimes) {
        total += time;
    }
    cout << "Average runtime: " << total / runTimes.size() << " ms" << endl;
}

/*
int main(int argc, char** argv){
    chrono::steady_clock::time_point begin = chrono::steady_clock::now(); // Starting time

    vector<bool> primes((MAX_PRIME/2 - 1), false);  // (3, end] inclusive.
    wheelFactorization(primes);
    sieveVector(primes);

    long long time = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin).count(); // Ending time
    cout << "runtime: " << time << " ms" << endl;

    ofstream file("file.txt");

    int count = 1;  // 2 is prime.
    for(int i = 0; i < primes.size(); i++){
        if(primes[i]){
            count++;
        }
    }
    file << endl << "Total primes: " << count << endl;
}
*/