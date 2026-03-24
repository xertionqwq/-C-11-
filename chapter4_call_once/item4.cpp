#include<iostream>
#include<thread>
#include<mutex>
#include<string>
static std::once_flag once;
// 单例模式的日志
class Log {
    Log() = default;
    static Log *log;
public:
    Log(const Log &log) = delete;
    Log &operator=(const Log &log) = delete;

    static Log& getInstance() {
        // static Log log;//懒汉模式
        // return log;

        // static Log *log = nullptr; //饿汉模式
        // if (!log) {
        //     log = new Log();
        // }
        // return *log;

        // call_once仅能在线程里面使用
        std::call_once(once, [](){
            if(!log) log = new Log; });
        return *log;
    }
    // static void init(){
    //     if (!log){
    //         log = new Log;
    //     }
    // }
    void PrintLog(std::string message) {
        std::cout << __TIME__ << message << std::endl;
    }
};
Log* Log::log = nullptr; 

void printError(){
    Log::getInstance().PrintLog("error");
}

int main() {
    std::thread t1(printError);
    std::thread t2(printError);
    t1.join();
    t2.join();

    std::cout << "return done" << std::endl;
    return 0;
}