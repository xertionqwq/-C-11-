#include<string>
#include<iostream>
#include<thread>
#include<future>
// async用于异步执行函数，返回future对象，表示异步操作结果
// 避免了手动创建线程和管理线程

int func() {
    int i = 0;
    for (; i < 10; ++i)
    {
        std::cout << "func: " << i << std::endl;
    }
    return i;
}

// pacaged_task为类模板，将可调用对象封装为异步操作
// 返回std::future对象，表示异步操作的结果
// 可以得到线程操作的返回值 
void test1() {
    std::packaged_task<int()> task(func);
    auto future_result = task.get_future();
    std::thread t1(std::move(task)); // 需要手动开辟线程
    std::cout << "func() begin" << std::endl;
    std::cout << func() << std::endl;
    t1.join();

    // 只有在func()执行完后才有future的值
    std::cout << future_result.get() << std::endl; 
}

// promise为类模板，在线程中产生一个值，并可在另一个线程中获得
// promise常与future和async一起使用，实现异步编程
void test2() {
}
void set(std::promise<int> &share) {
    share.set_value(1000);
}

int main() {
    // std::future<int> future_result = std::async(std::launch::async, func);
    // std::cout << "func() begin" << std::endl;
    // std::cout << func() << std::endl;

    // 只有在func()执行完后才有future的值
    // std::cout << future_result.get() << std::endl;

    std::promise<int> share;    //不可拷贝对象
    auto future_result = share.get_future();
    std::thread t1(set, std::ref(share));
    t1.join();

    std::cout << future_result.get() << std::endl;

    std::cout << "return done" << std::endl;
    return 0;
}