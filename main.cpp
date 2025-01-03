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
    for(int j = value * value; j < sqrt(MAX_PRIME); j += (value * 2)){ // Skip even numbers
        primes[(j-3)/2] = false;
    }
}

void sieveVector(vector<bool> &primes){
    
    int limit = sqrt(MAX_PRIME) / 2;  // Avoid recalculating every run
    for(int i = 0; i < limit; i++){
        if(primes[i]){
            THREAD_POOL.detach_task([=, &primes] {
                sieveValue(primes, i);
            });
            THREAD_POOL.wait();
        }
    }
}

/*void segmentedSieve(int value, int startingIndex, int endingIndex, vector<bool> &primes) {
    endingIndex = min(endingIndex, MAX_PRIME);
    if(startingIndex % 2 == 0){ 
        printf("Odd found: %d, the value is %d\n", startingIndex, value);
        startingIndex -= value;
    }
    for(int i = startingIndex; i <= endingIndex; i+=(value*2)) { // * 2 so it iterates through odd numbers only
        primes[(i-3)/2] = false;
    }
}

void sieveVector(vector<bool> &primes){
    int runs = ceil(sqrt(MAX_PRIME));

    for(int i = 0; i < runs; i++) {
        if(primes[i]) {
            int value = (i*2)+3;
            int start = value * value;
            int end = MAX_PRIME;
            int range = end - start;
            int workload = range / MAX_THREADS;

            int lastStartingValue = start;

            for(int j = 0; j < MAX_THREADS; j++) {
                // Split workload:        
                int endValue = (lastStartingValue + workload);
                THREAD_POOL.detach_task([=, &primes] {  
                    segmentedSieve(value, lastStartingValue, endValue, ref(primes));
                });
                lastStartingValue = endValue;
            }
        }
        THREAD_POOL.wait();
    }
}*/

void individualWheelValue(int startValue, int endValue){
    vector<int> offsets = {4, 2, 4,  2,  4,  6,  2,  6};
    unordered_map<int, int> wheelLookup = {{1, 0}, {7, 1}, {11, 2}, {13, 3}, {17, 4}, {19, 5}, {23, 6}, {29, 7}}; // to get the index of the offset
    vector<int> wheel;

    int value = startValue;
    while (wheelLookup.count(value % 30) == 0) { value++; }
    int index = wheelLookup[value % 30] + 7;
    while(value < endValue){
        cout << value % 30 << ", ";
        wheel.push_back(value);
        value += offsets[index % 8];
        index++;
    }
}

void wheelFactorization(){

    individualWheelValue(0, 1000);
    individualWheelValue(1000, 2000);


}

void run(vector<long long> &runTimes){
    chrono::steady_clock::time_point begin = chrono::steady_clock::now(); // Starting time

    wheelFactorization();/*
    sieveVector(wheel);

    long long time = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin).count(); // Ending time
    cout << "runtime: " << time << " ms" << endl;
    runTimes.push_back(time);

    ofstream file("file.txt");

    int count = 1;  // 2 is prime.
    for(int i = 0; i < primes.size(); i++){
        if(primes[i]){
            count++;

        }
    }
    file << endl << "Total primes: " << count << endl;*/
    
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