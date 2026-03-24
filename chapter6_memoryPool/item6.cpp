#include<iostream>
#include<thread>
#include<unistd.h>
#include<chrono>
#include<condition_variable>
#include<functional>
#include<vector>
#include<queue>
#include<mutex>
#include<cstdio>

class ThreadPool{
    std::vector<std::thread> threads;   //线程池
    std::queue<std::function<void()>> tasks;//任务队列

    std::mutex mtx;//互斥锁
    std::condition_variable condition;//条件变量

    bool stop;      //线程池停止变量

public:
    ThreadPool(int numThreads) : stop(false) {
        for (int i = 0; i < numThreads; ++i) {
            // 消费者处理线程
            // 使用匿名函数,捕获this指针
            threads.emplace_back([this]{
                while(1) {
                    // 拿到任务的整个过程要上锁
                    std::unique_lock<std::mutex> lock(mtx);
                    // 条件变量，当任务队列为空，或者不再发出任务时阻塞
                    condition.wait(lock, [this]
                                   { return !tasks.empty() || stop; });
                    
                    // 线程已终止且不再有任务
                    if (stop && tasks.empty())
                        return;

                    //从任务队列中取出任务
                    std::function<void()> task{std::move(tasks.front())};
                    tasks.pop();//该任务离开任务队列
                    lock.unlock();//解锁，方便其他线程获取任务
                    task(); // 进行任务
                } }
            );
        }
    }
    ~ThreadPool() {
        {
            // 限定作用域，结束后自动解锁
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }
        //通知所有线程结束工作
        condition.notify_all();
        for (auto &thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }// 等待线程结束工作
         // 所有任务完成，销毁线程池
    }

    template<typename F, typename ... Args> //不确定参数有多少个，使用模板
    //&&万能引用
    void enqueue(F &&f, Args&&... args) {
        //std::bind()使用函数绑定器
        std::function<void()> task = std::bind(
            std::forward<F>(f),          // 转发函数对象（必须指定模板参数）
            std::forward<Args>(args)...  // 转发所有参数（逐个指定模板参数）
        );//由于使用了万能引用，所以要用完美转发保留表达式类型

        // 对任务队列开始操作，上锁
        {
            std::unique_lock<std::mutex> lock(mtx);
            tasks.emplace(std::move(task));//move()触发移动构造，防止深拷贝
        }
        condition.notify_one();//通知线程来取任务
    }
};

int main() {
    ThreadPool pool(5);//开辟5个线程的线程池

    for (int i = 0; i < 10; ++i) {
        pool.enqueue([i]()
                     {  printf("task : %d is running\n", i);
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        printf("task : %d is done\n", i); }
                    );
    }
    for (int i = 0; i < 10; ++i) {
        pool.enqueue([i]()
                     {  printf("task : %d is running\n", i);
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        printf("task : %d is done\n", i); }
                    ); // 用printf()避免线程争夺io资源
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "return done" << std::endl;
    return 0;
}