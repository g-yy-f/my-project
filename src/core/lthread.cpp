#include "lthread.h"

#include <condition_variable>
#include <atomic>
#include <thread>
#include <mutex>

#define SIZE (10000)

using namespace lmc;
using namespace std;

/**
 * Thread::Impl - PIMPL 实现细节
 *
 * 成员说明：
 * - c: 用于线程等待与唤醒的条件变量
 * - bNotify: 通知标志，防止虚假唤醒；只有为 true 时，条件变量等待才会结束并执行 run()
 * - bStop: 控制线程是否应继续运行；当 bStop 被置为 false 时，线程退出
 * - t: 实际运行的 std::thread
 */
class Thread::Impl {
public:
    std::condition_variable c;
    std::atomic<bool> bNotify; // 防止虚假唤醒
    std::atomic<bool> bStop;   // 线程是否停止（false 表示应退出线程）
    std::thread t;
};

/**
 * 构造函数：初始化控制标志，并启动后台线程。
 *
 * 后台线程循环逻辑：
 *  - 创建一个局部 mutex 并通过 unique_lock 上锁，以便与 condition_variable 协作
 *  - 使用 wait(predicate) 等待 bNotify 变为 true（避免虚假唤醒）
 *  - 如果 bStop 为 false，则退出线程（return）
 *  - 否则调用派生类的 run() 执行一次任务逻辑
 */
Thread::Thread() : pImpl(std::make_unique<Impl>()) {
    pImpl->bStop = true;
    pImpl->bNotify = false;

    pImpl->t = thread([this] {
        mutex localMutex;
        unique_lock<mutex> localLock(localMutex);
        while (1){
            // 等待通知（bNotify 为 true 时继续）
            pImpl->c.wait(localLock, [this] {
                return pImpl->bNotify.load();
            });

            // 当 bStop 被置为 false（说明需要退出），线程返回并结束
            if (!pImpl->bStop)
                return;

            // 被唤醒后执行派生类的 run() 一次工作逻辑
            run();
        }
    });
}

Thread::~Thread() {}

/**
 * 停止通知：将 bNotify 置为 false，使条件变量等待在下一轮阻塞。
 * 注意：这并不会直接退出线程，线程仍然存在；若需要销毁线程请调用 destory()。
 */
void Thread::stop() {
    // 必须由用户控制虚假唤醒与通知逻辑，这里仅更新通知标志
    pImpl->bNotify.store(false);
}

/**
 * 唤醒线程：设置通知标志并通知条件变量，后台线程会在条件满足时被唤醒并执行 run()
 */
void Thread::start() {
    pImpl->bNotify.store(true);
    pImpl->c.notify_one();
}

/**
 * 销毁线程：将 bStop 设置为 false，表示线程应退出；随后调用 start() 唤醒线程以便其能检测到 bStop，
 * 最后 join 后台线程以回收资源。
 *
 * 这是一个阻塞调用（直到线程退出并 join）。
 */
void Thread::destory() {
    pImpl->bStop = false;
    start();
    pImpl->t.join();
}