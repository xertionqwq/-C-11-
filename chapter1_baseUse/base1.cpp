#include<iostream>
#include<thread>
#include<string>   

// 进程: 运行中的程序
// 线程: 进程中的进程

void fun(std::string s) {
    for (int i = 0; i < 10; ++i) {
        std::cout << s << std::endl;
    }
}

void fun1() {
    for (int i = 0; i < 50; ++i) {
        std::cout << "fun1" << std::endl;
    }
}

int main() {
    std::thread t1(fun, "hello world");//主程序创建一个子线程t1，t1执行fun函数，主程序继续往下执行，不会等待
    bool isJoin = t1.joinable();//判断t1是否可以被join
    if (isJoin) {
        t1.join();//主程序等待t1执行完毕后，才继续往下执行
        // t1.detach();//主程序和t1分离，主程序继续往下执行，t1在后台继续执行
        // fun1();
    }
    fun1();
    std::cout << "return done" << std::endl;
    return 0;
}