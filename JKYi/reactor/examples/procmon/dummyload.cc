#include"JKYi/atomic.h"
#include"JKYi/condition.h"
#include"JKYi/reactor/CurrentThread.h"
#include"JKYi/mutex.h"
#include"JKYi/reactor/Thread.h"
#include"JKYi/timestamp.h"
#include"JKYi/reactor/EventLoop.h"

#include<math.h>
#include<stdio.h>

using namespace JKYi;
using namespace JKYi::net;

int g_cycles = 0;    //循环的次数
int g_percent = 82;  //目标百分比，也就是需要讲cpu维持到的百分比
AtomicInt32 g_done;  //用来控制子线程是否退出
bool g_busy = false;
Mutex g_mutex;
Condition g_cond(g_mutex);

//忙碌状态下应该调用的函数,也就是通过计算平方来占用CPU就ok了
double busy(int cycles){
    double result = 0;
    for(int i = 0;i < cycles;++i){
        result += sqrt(i) * sqrt(i + 1);
    }
    return result;
}

double getSeconds(int cycles){
    Timestamp start = Timestamp::now();
    busy(cycles);
    return timeDifference(Timestamp::now(),start);
}

void findCycles(){
    g_cycles = 1000;
    while(getSeconds(g_cycles) < 0.001){
        g_cycles = g_cycles + g_cycles / 4;
    }
    printf("cycles = %d\n",g_cycles);
}

void threadFunc(){
    while(g_done.get() == 0){
        {
            Mutex::Lock lock(g_mutex);
            while(!g_busy){
                g_cond.wait();
            }
            busy(g_cycles);
        }
        //printf("thread exit\n");
    }
}

//该函数的作用就是将当前这一秒的cpu使用率维持在percent
void load(int percent){
    percent = std::max(0,percent);
    percent = std::min(100,percent);

    //使用的是 Bresenham's line algorithm
    int err = 2 * percent - 100;
    int count = 0;
    //将1秒分为100份
    for(int i = 0;i < 100;++i){
        bool busy = false;
        if(err > 0){
            busy = true;
            err += 2 * (percent - 100);
            ++count;
        }else{
            err += 2 * percent;
        }

        {
            Mutex::Lock lock(g_mutex);
            g_busy = busy;
            g_cond.notifyAll();
        }
        CurrentThread::sleepUsec(10 * 1000);
    }
    assert(count == percent);
}


void fixed(){
    while(true){
        load(g_percent);
    }
    return ;
}
void cosine(){
    while(true){
        //200s作为一个周期
        for(int i = 0;i < 200;++i){
            int percent = static_cast<int>((1.0 + cos(i * 3.14159 / 100)) / 2 * g_percent + 0.5);
            load(percent);
        }
    }
}

void sawtooth(){
    while(true){
        for(int i = 0;i <= 100;++i){
            int percent = static_cast<int>(i / 100.0 * g_percent);
            load(percent);
        }
    }
}

int main(int argc,char** argv){
    if(argc < 2){
        printf("Usage: %s [fctsz] [percent] [num_threads]\n",argv[0]);
        return 0;
    }
    printf("pid %d\n",getpid());
    findCycles();

    g_percent = argc > 2 ? atoi(argv[2]) : 43;
    int numThreads = argc > 3 ? atoi(argv[3]) : 1;
    std::vector<std::unique_ptr<JKYi::net::Thread>> threads;
    for(int i = 0;i < numThreads;++i){
        threads.emplace_back(new JKYi::net::Thread(threadFunc));
        threads.back()->start();
    }
    switch(argv[1][0]){
        case 'f':
            fixed();
            break;
        case 'c':
            cosine();
            break;
        case 'z':
            sawtooth();
            break;
        default:
            break;
    }
    //将所有的子线程给唤醒，然后回收
    g_done.getAndSet(1);
    {
        Mutex::Lock lock(g_mutex);
        g_busy = true;
        g_cond.notify();
    }
    for(int i = 0;i < numThreads;++i){
        threads[i]->join();
    }
    return 0;
}

