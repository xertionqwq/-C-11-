#include <iostream>
#include <thread>
#include <atomic>

std::atomic<int> shared_data(0); // 原子变量，保证线程安全
void fun()
{
    for (int i = 0; i < 3000000; ++i) {
        // 效率更高的原子操作，避免了锁的开销
        shared_data++;
    }
}

int main() {
    auto last = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
    shared_data.store(50);
    std::thread t1(fun);
    std::thread t2(fun);
    t1.join();
    t2.join();
    std::cout << "shared_data: " << shared_data.exchange(1e5) << std::endl;
    std::cout << "shared_data: " << shared_data << std::endl;

    auto cur = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
    std::cout << "time: " << cur - last << "ms" << std::endl;

    std::cout << "return done" << std::endl;
    return 0;
}