#include "workqueue.h"

using namespace lmc;

/**
 * 设置互斥类型（应在构造时或使用前设置）
 */
void SMutex::setMutexType(MutexType m) {
    mMutexType = m;
}

/**
 * lock()/unlock() 根据配置的互斥类型执行不同操作：
 * - MutexType::Mutex -> std::mutex
 * - MutexType::Spin  -> 自旋锁
 * - MutexType::None  -> 不做任何操作（调用者需保证并发安全）
 */
void SMutex::lock() {
    switch (mMutexType) {
        case MutexType::Mutex:
            mMutex.lock();
        break;
        case MutexType::Spin:
            mSpinMutex.lock();
        break;
        case MutexType::None:
        return;
        break;
    }
}

void SMutex::unlock() {
    switch (mMutexType){
        case MutexType::Mutex:
            mMutex.unlock();
        break;
        case MutexType::Spin:
            mSpinMutex.unlock();
        break;
        case MutexType::None:
        return;
        break;
    }
}

/**
 * 构造函数：设置互斥类型
 */
WorkQueue::WorkQueue(MutexType m) {
    mutex.setMutexType(m);
}

/**
 * 析构函数：
 * - 在析构时先对队列加锁并清空队列
 * - 然后调用基类的 destory() 停止并 join 后台线程，确保派生类析构时不会因为基类线程调用纯虚函数导致崩溃
 * - 最后释放互斥锁
 */
WorkQueue::~WorkQueue() {
    mutex.lock();
    while (!workqueue.empty()) 
        workqueue.pop();

    // 将基类的线程销毁操作转交给派生类执行
    destory();
    mutex.unlock();
}

/**
 * run() 实现：从队列中取出一个任务并执行
 */
void WorkQueue::run() {
    mutex.lock();
    if (workqueue.empty()) {
        mutex.unlock();
        return;
    }

    auto f = move(workqueue.front());
    workqueue.pop();
    mutex.unlock();

    // 在不持锁的情况下执行任务，避免长期占用互斥体
    f();
}

/**
 * 对外提供的停止接口，内部委托给基类实现（仅取消通知，不直接销毁线程）
 */
void WorkQueue::stopWorkQueue() {
    stop();
}