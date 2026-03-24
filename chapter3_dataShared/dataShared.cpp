#include<memory>
#include<iostream>
#include<thread>
#include<mutex>//使用互斥锁

void add2(int &a) {
    a++;
    a++;
}
void add4(int &a) {
    int i = 3;
    while(i--) {
        a++;
    }
}

std::mutex mtx;
void add(int &a)
{
    mtx.lock();//锁之间的片段只有一个线程进行访问
    for (int i = 0; i < 2000000; i++) { 
        a++; 
    }
    mtx.unlock();//频繁加解锁会增加耗时
}

int main() {
    int a = 0;
    std::thread t2(add, std::ref(a));
    std::thread t4(add, std::ref(a));//t2,t4出现数据竞争
    t2.join();
    t4.join();
    std::cout << "a: " << a << std::endl;//如果不上锁，结果并非预期

    std::cout << "return done" << std::endl;
    return 0;
}