#include<memory>
#include<iostream>
#include<thread>
#include<mutex>//使用互斥锁
#include<unistd.h>

//两个线程t1，t2
//t1先获取a,再获取b
//t2先获取b，再获取a
//由于a，b同时上锁，程序锁死
std::mutex m1, m2;
void func1()
{
    m1.lock();// t1先拿到m1
    sleep(3);
    m2.lock();// t1想拿m2加锁, 但m2现在在t2手上
    m1.unlock();
    m2.unlock();
}
void func2() {
    m2.lock();// 由于t1在sleep, t2也拿到了m2
    m1.lock();// t2想拿m1加锁, 但m1现在在t1手上, 程序死锁
    m2.unlock();
    m1.unlock();
}

int main(){
    std::thread t1(func1);
    std::thread t2(func2);
    t1.join();
    t2.join();

    std::cout << "return done" << std::endl;
    return 0;
}