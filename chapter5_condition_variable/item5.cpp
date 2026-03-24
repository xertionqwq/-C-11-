#include<iostream>
#include<unistd.h>
#include<thread>
#include<string>
#include<condition_variable>//条件变量
#include<queue> //任务队列
#include<mutex>

// 生产者和消费者模型
// 使用std::condition_variable进行通知
// notify_one()通知一个线程取任务
// notify_all()通知所有线程
std::queue<int> q_queue;
std::condition_variable g_cv;// 条件变量（用于线程通知/等待）
std::mutex mtx;              // 互斥锁（保护队列）
bool is_producer_done = false;

// 操作同一个对象q_queue,所有要加锁
// 派发任务
void Producer() {
    for (int i = 0; i < 10; i++) {
        {
            // 作用域：限制锁的持有时间，避免长时间占用锁
            std::unique_lock<std::mutex> lock(mtx);
            q_queue.push(i);
            // 通知一个线程来取任务
            g_cv.notify_one();
            std::cout << "Producer add: " << i << std::endl;
        }
        sleep(0.5);
    }
    is_producer_done = true;
}

// 完成任务
void Consumer() {
    while(true) {
        std::unique_lock<std::mutex> lock(mtx);
        // 如果队列为空，则需要等待
        // 第二个参数要用仿函数实现
        // 第二个参数为false时，线程阻塞在这里
        // 直到生产队列传来消息，继续消费
        g_cv.wait(lock, []
                  { return !q_queue.empty() || is_producer_done; });
        int value = q_queue.front();
        q_queue.pop();

        std::cout << "Consumer remove: " << value << std::endl;
    }
}

int main(){
    std::thread t1(Producer);
    std::thread t2(Consumer);
    std::thread t3(Consumer);
    t1.join();
    t2.join();
    t3.join();

    std::cout << "return done" << std::endl;
    return 0;
}