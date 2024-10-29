#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <mutex>
#include <fstream>
#include <chrono>

using namespace std;
mutex mtx;

void threadFunction(int value, vector<bool>* values) {
    for(int i = value * value; i < values->size(); i+=value) {
        if(values->at(i)) {
            values->at(i) = false;
        }
    }
}

void invokeThreads(int numPrimes, int numThreads, vector<bool>* values) {
    vector<thread> threads;
    
    for(int i = 2; i < sqrt(numPrimes); i++) {
        if(values->at(i)) {
            threads.push_back(thread(threadFunction, i, values));
        }
    }
    
    for(auto& th : threads) {
        th.join();
    }
}

vector<int> findPrimeValues(int numPrimes, int threads) {
    vector<bool> values(numPrimes, true);
    vector<int> primes;
    values.at(0) = false; values.at(1) = false;

    invokeThreads(numPrimes, 8, &values);

    for(int i = 0; i < numPrimes; i++) {
        if(values.at(i)) {
            primes.push_back(i);
        }
    }
    return primes;
}

int main(int argc, char* argv[]) {
    int numPrimes = 100000000;
    int numThreads = 8;
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    ofstream myFile("file.txt");

    vector<int> primes = findPrimeValues(numPrimes, numThreads);
    /*for(int i = 0; i < primes.size(); i++) {
        myFile << primes.at(i) << ", ";
    }*/
    myFile << "Count: " << primes.size() << endl;
    myFile << "Time: " << chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin).count() << " ms" << endl;
    myFile.close();
    return 0;
}
