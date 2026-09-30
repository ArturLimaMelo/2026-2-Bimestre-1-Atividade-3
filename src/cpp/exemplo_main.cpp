#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

using namespace std;

void worker(int id) {
    cout << "Worker " << id << " starting\n";
    this_thread::sleep_for(chrono::milliseconds(100 * id));
    cout << "Worker " << id << " done\n";
}

int main() {
    cout << "Threads demo" << endl;
    vector<thread> threads;
    for (int i = 1; i <= 4; ++i) {
        threads.emplace_back(worker, i);
    }
    for (auto &t : threads) t.join();
    return 0;
}
