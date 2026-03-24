#include<iostream>
#include<memory> // 用于智能指针
#include<thread>
#include <unistd.h>

// 1.传递临时变量,直接报错
void foo(int &x) {
    x += 1;
}

// 2.传递指针指向局部变量
std::thread tt;
void test() {
    int a = 2;
    tt = std::thread(foo, std::ref(a));//a的地址已经清除
}

void func(int *p) {
    sleep(2);
    for (int i = 0; i < 10; i++)
    {
        std::cout << *p << std::endl;
    }
}

class A{
public:
    A() = default;
    void Afun()
    {
        std::cout << "A::Afun()" << std::endl;
    }
private:
    friend void thread_foo();//使用友原函数
    void pvtfoo()
    {
        std::cout << "A::pvtfoo()" << std::endl;
    }
};
void thread_foo() {
    std::shared_ptr<A> rr = std::make_shared<A>();
    std::thread dd(&A::pvtfoo, rr);
    dd.join();
}

int main() {
    // test();
    // tt.join();
    // std::thread t(foo, 1);
    int a = 1;
    // std::thread t(foo, a);
    std::thread t(foo, std::ref(a));//使用std:ref()转为引用
    t.join();
    std::cout << a << std::endl;


    // 3.子程序还未运行完，主程序指针已被释放
    // int *ptr = new int(91);
    // std::thread mm(func, ptr);
    // delete ptr;
    // mm.join();

    // 4.类对象成员函数作为入口，但是成员被提前释放
    // A classa;
    // std::shared_ptr<A> aa = std::make_shared<A>();//使用智能指针可以解决
    // std::thread nn(&A::Afun, aa);
    // nn.join();

    // 5.入口函数为类的私有函数
    thread_foo();

    std::cout << "return done" << std::endl;
    return 0;
}