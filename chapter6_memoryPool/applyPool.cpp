#include<iostream>
#include<thread>
#include<chrono>
#include<condition_variable>
#include<functional>
#include<vector>
#include<queue>
#include<mutex>
#include<cstdio>
#include<memory>
#include<string>
#include<atomic>
#include<random>

// ===== 单例线程池（修复析构和输出问题）=====
class ThreadPool{
private:
    // 友元声明：让自定义删除器能访问私有析构函数
    friend struct ThreadPoolDeleter;

    ThreadPool(int numThreads = 8) : stop(false), completedTasks(0), failedTasks(0) {
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back([this]{
                while(1) {
                    std::unique_lock<std::mutex> lock(mtx);
                    condition.wait(lock, [this] {
                        return !tasks.empty() || stop;
                    });
                    
                    if (stop && tasks.empty()) return;

                    std::function<void()> task{std::move(tasks.front())};
                    tasks.pop();
                    lock.unlock();
                    
                    try {
                        task(); // 执行任务
                    } catch (...) {
                        failedTasks++; // 捕获所有异常，统计失败数
                    }
                }
            });
        }
    }

    // 私有析构函数（仅自定义删除器可访问）
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &thread : threads) {
            if (thread.joinable()) thread.join();
        }
        printf("ThreadPool destroyed: completed=%d, failed=%d\n", completedTasks.load(), failedTasks.load());
    }

    // 禁用拷贝/移动
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    // 成员变量
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable condition;
    bool stop;

    // 任务统计（原子变量，线程安全）
    std::atomic<int> completedTasks;
    std::atomic<int> failedTasks;

    // 单例管理：自定义删除器
    struct ThreadPoolDeleter {
        void operator()(ThreadPool* ptr) const {
            delete ptr; // 能访问私有析构函数
        }
    };
    static std::mutex singleton_mtx;
    // 使用自定义删除器的unique_ptr
    static std::unique_ptr<ThreadPool, ThreadPoolDeleter> instance;

public:
    // 获取单例（默认8线程）
    static ThreadPool& getInstance(int numThreads = 8) {
        if (!instance) {
            std::lock_guard<std::mutex> lock(singleton_mtx);
            if (!instance) {
                instance.reset(new ThreadPool(numThreads));
            }
        }
        return *instance;
    }

    // 提交任务（添加队列容量限制）
    template<typename F, typename ... Args>
    bool enqueue(F &&f, Args&&... args) {
        // 绑定任务和参数
        auto task = std::bind(
            [this, f = std::forward<F>(f)](Args&&... args) mutable {
                f(std::forward<Args>(args)...);
                this->completedTasks++; // 任务完成计数
            },
            std::forward<Args>(args)...
        );

        std::unique_lock<std::mutex> lock(mtx);
        // 队列最大容量限制（防止内存爆炸）
        const size_t MAX_QUEUE_SIZE = 10000;
        if (tasks.size() >= MAX_QUEUE_SIZE) {
            printf("Task queue full! Reject task\n");
            return false;
        }

        tasks.emplace(std::move(task));
        lock.unlock();
        condition.notify_one();
        return true;
    }

    // 获取任务统计
    int getCompletedTasks() const { return completedTasks.load(); }
    int getFailedTasks() const { return failedTasks.load(); }

    // 优雅关闭线程池
    static void shutdown() {
        if (instance) {
            std::lock_guard<std::mutex> lock(singleton_mtx);
            instance.reset(); // 调用自定义删除器，销毁实例
        }
    }
};

// 静态成员初始化（使用自定义删除器）
std::mutex ThreadPool::singleton_mtx;
std::unique_ptr<ThreadPool, ThreadPool::ThreadPoolDeleter> ThreadPool::instance = nullptr;

// ===== 工具函数：将thread::id转换为数值（解决输出问题）=====
uint64_t threadIdToUint64(std::thread::id id) {
    // 将thread::id转换为字符串，再转为数值
    std::string idStr = std::to_string(std::hash<std::thread::id>{}(id));
    return std::stoull(idStr);
}

// ===== 高并发场景：模拟HTTP请求处理 =====
// 1. 模拟数据库查询（耗时IO操作）
std::string queryDB(int reqId) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
    return "user_data_" + std::to_string(reqId);
}

// 2. 模拟HTTP请求处理函数（修复输出问题）
void handleHttpRequest(int reqId) {
    try {
        // 修复：将thread::id转为数值后输出
        uint64_t tid = threadIdToUint64(std::this_thread::get_id());
        printf("[Req %d] Start processing (thread: %lu)\n", reqId, tid);
        
        // 核心业务：查询数据库
        std::string data = queryDB(reqId);
        
        // 模拟业务计算
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        printf("[Req %d] Done: %s (thread: %lu)\n", reqId, data.c_str(), tid);
    } catch (const std::exception& e) {
        uint64_t tid = threadIdToUint64(std::this_thread::get_id());
        printf("[Req %d] Error: %s (thread: %lu)\n", reqId, e.what(), tid);
        throw; // 抛出异常，让线程池统计失败数
    }
}

// 3. 模拟高并发请求生成器
void generateHighConcurrencyRequests(int totalRequests) {
    ThreadPool& pool = ThreadPool::getInstance(8); // 8线程
    
    // 生成大量请求
    for (int i = 0; i < totalRequests; ++i) {
        // 提交任务（检查是否成功）
        if (!pool.enqueue(handleHttpRequest, i)) {
            printf("Failed to submit req %d (queue full)\n", i);
        }
        
        // 模拟请求到达速率（每秒1000个）
        if (i % 100 == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    printf("All %d requests submitted to thread pool\n", totalRequests);
}

// ===== 主函数 =====
int main() {
    const int TOTAL_REQUESTS = 10000; // 1万个并发请求
    
    // 启动请求生成器
    std::thread requestGenerator(generateHighConcurrencyRequests, TOTAL_REQUESTS);
    requestGenerator.join();
    
    // 等待所有任务处理完毕
    printf("Waiting for all requests to finish...\n");
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // 统计结果
    ThreadPool& pool = ThreadPool::getInstance();
    printf("Total completed tasks: %d\n", pool.getCompletedTasks());
    printf("Total failed tasks: %d\n", pool.getFailedTasks());
    
    // 优雅关闭线程池
    ThreadPool::shutdown();
    
    printf("Program exit successfully\n");
    return 0;
}