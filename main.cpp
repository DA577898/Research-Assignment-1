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
const int MAX_PRIME = 10000;
BS::thread_pool THREAD_POOL(MAX_THREADS);


/**
 * This program calculates the prime numbers up to a given number using the Sieve of Eratosthenes algorithm.
 * 
 * The program uses a multithreaded approach to calculate the prime numbers.
 * https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes
 */





/*void sieveValue(vector<bool> &primes, int i){
    int value = (i*2)+3;
    for(int j = value; j < sqrt(MAX_PRIME); j += (value * 2)){ // Skip even numbers
        primes[(j-3)/2] = false;
    }
}*/

void threadFunction(int value, int startingIndex, int endingIndex, vector<bool> &values) {
    vector<int> sequence = {1, 7, 11, 13, 17, 19, 23, 29};
    
    for (int i = startingIndex; i < endingIndex; i += value * 2) {
        values[i] = false;
    }
}

void invokeThreads(int numPrimes, int numThreads, vector<bool> &values) {
    int runs = ceil(sqrt(numPrimes));

    for (int i = 3; i < runs; i += 2) {  // Go only through odd numbers (evens are all non-prime and code below accounts for it)
        if (values[i]) {
            int start = i * i;
            int end = numPrimes;
            int range = end - start;
            int workload = range / numThreads;

            for (int j = 0; j < numThreads; j++) {
                int lastStartingValue = start + j * workload;
                int endValue = min(start + (j + 1) * workload, end);
                THREAD_POOL.detach_task([=, &values] {
                    threadFunction(i, lastStartingValue, endValue, ref(values));
                });
            }
        }
    }
    THREAD_POOL.wait();
}

void sieveValue(vector<bool> &primes, int i){
    int value = (i*2)+3;
    for(int j = value * value; j < MAX_PRIME; j += (value * 2)){ // Skip even numbers
        primes[(j-3)/2] = false;
    }
}

void sieveVector(vector<vector<bool>> &wheel){
    vector<bool> primes = {wheel[0].begin(), wheel[0].begin() + sqrt(MAX_PRIME) / 2 - 1};
/*     for(int i = 0; i < primes.size(); i++){
        if(primes[i]){
            cout << "Sieve value: " << 2 * i + 3 << endl;
        }
    } */
/*
    primes[0] = true;  // 3 is prime.
    primes[1] = true;  // 5 is prime.

    for(auto value : wheel[0]){
        if(value > sqrt(MAX_PRIME)){break;}
        primes[(value-3)/2] = true;
    }

    invokeThreads(10000, MAX_THREADS, primes);

    int count = 1;  // 2 is prime.
    for (bool prime : primes) {
        if (prime) {
            count++;
        }
    }
    cout << "\nTotal primes: " << count << endl;
    

    
        vector<future<vector<int>>> futures;
    futures.reserve(MAX_THREADS);

    vector<bool> primes((MAX_PRIME/2) - 1, false);
    primes[0] = true;  // 3 is prime.
    primes[1] = true;  // 5 is prime.

    for (auto vec : wheel) {
        for (int num : vec) {
            primes[(num/2) - 1] = true;
        }
    }

    for(int i = 0; i < primes.size(); i++){
        if(primes[i]){
            sieveValue(primes, i);
        }
    }

    int count = 1;  // 2 is prime.
    for (bool prime : primes) {
        if (prime) {
            count++;
        }
    }
    cout << "\nTotal primes: " << count;
    cout << endl;*/
}
    



vector<bool> individualWheelValue(int startValue, int endValue, int chunkNumber){
    vector<int> offsets = {4, 2, 4,  2,  4,  6,  2,  6};
    unordered_map<int, int> wheelLookup = {{1, 0}, {7, 1}, {11, 2}, {13, 3}, {17, 4}, {19, 5}, {23, 6}, {29, 7}}; // to get the index of the offset
    vector<bool> wheel((endValue - startValue)/2 + 1, false);

    int value = startValue;
    while (wheelLookup.count(value % 30) == 0) { value++; }  // find next value that will be part of the wheel
    int index = wheelLookup[value % 30] + 7;

    ofstream fileOne;
    fileOne.open("fileOne.txt", ios::app);
    while(value < endValue){  // calculate the wheel values
        wheel[(value - startValue) / 2] = true;
        fileOne << "Thread " << chunkNumber << " value: " << value << " index: " << (value - startValue) / 2 << endl;
        value += offsets[index % 8];
        index++;
    }
    fileOne.close();
    return wheel;
}

void wheelFactorization(vector<vector<bool>> &wheel){
    vector<future<vector<bool>>> futures;
    futures.reserve(MAX_THREADS);
    wheel.reserve(MAX_THREADS);

    remove("fileOne.txt");

    for(int i = 0; i < MAX_THREADS; i++){
        int start = i * (MAX_PRIME / MAX_THREADS);
        int end = min((i + 1) * (MAX_PRIME / MAX_THREADS), MAX_PRIME);
        futures.push_back(THREAD_POOL.submit_task([=] () {
            return individualWheelValue(start, end, i);
        }));
        THREAD_POOL.wait();

    }
    THREAD_POOL.wait();

    for(auto &future : futures){
        wheel.push_back(future.get());
    }
    //wheel[0][1] = true;  // 3 is prime.
    //wheel[0][2] = true;  // 5 is prime;
    int count = 1;  // 2 is prime.
    ofstream fileTwo("fileTwo.txt");
    for (int i = 0; i < wheel.size(); i++) {
        for (int j = 0; j < wheel[i].size(); j++) {
            if (wheel[i][j]) {
                fileTwo << "Thread " << i << " value: " << ((i * (MAX_PRIME / MAX_THREADS)) + (2 * j)) + 1 << " index: " << j << endl;
                count++;
            }
        }
    }
    fileTwo.close();
    cout << endl << count << endl;
}

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
    for(int i = 0; i < 1; i++){run(runTimes);}
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