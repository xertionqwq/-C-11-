#include<iostream>
#include<unistd.h>
#include<thread>
#include<mutex>

int shared_data = 0;
std::mutex mtx;
std::timed_mutex tmtx;
// lock_guard,自动加解锁
// 构造函数调用，互斥量自动锁定
// 析构函数调用，互斥量自动解锁
// lock_guard对象无法移动和复制，所以只在局部作用域中使用

//unique_lock,方法更多，使用更灵活
// try_lock_for() 延迟加锁
// try_lock_until() 等待到指定时间

void func() {
    for (int i = 0; i < 2000000; i++) {
        // std::lock_guard<std::mutex> lg(mtx);
        // std::unique_lock<std::mutex> lg1(mtx);
        // lg1.lock(); // 手动加锁
        // shared_data++;

        std::unique_lock<std::timed_mutex> lg(tmtx, std::defer_lock);
        // 如果等待2秒仍然拿不到，则直接跳过，防止死锁
        if (lg.try_lock_for(std::chrono::seconds(2))) {
            sleep(4);
            shared_data++;
        }
    }
}

int main() {
    std::thread t1(func);
    std::thread t2(func);
    t1.join();
    t2.join();

    std::cout << shared_data << std::endl;
    return 0;
}