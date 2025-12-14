#ifndef THREAD_H
#define THREAD_H

#include <memory>

namespace lmc {

/**
 * Thread - 抽象线程基类
 *
 * 该类封装了一个后台线程的生命周期控制，派生类需要重载纯虚函数 run()
 * 以实现每次被唤醒时需要执行的逻辑。基类通过 pImpl 隐藏实现细节（condition_variable,
 * atomic 等），并提供 start()/stop()/destory() 接口供派生类或使用者控制线程的唤醒与终止。
 *
 * 重要约定：
 * - 后台线程会循环等待条件变量通知；被唤醒后会调用派生类实现的 run() 执行一次任务。
 * - 若需要彻底结束后台线程，请调用 destory()。
 * - 为避免派生类在析构期间被基类线程调用纯虚函数导致未定义行为，派生类应在析构中
 *   负责先调用 destory() 停止并 join 背景线程。
 */
class Thread {
public:
    Thread();
    virtual ~Thread();

    Thread(const Thread &) = delete;
    Thread(Thread &&) = delete;
    Thread &operator=(Thread &&) = delete;

protected:
    /**
     * 唤醒后台线程（设置通知标志并通知条件变量）
     * 派生类在有新任务或需要运行时调用该方法。
     */
    void start();

    /**
     * 停止“通知”标志，实际是否停止线程还需配合 destory 来执行线程退出。
     * stop() 主要用于控制是否继续被唤醒执行 run()，并不能直接终止线程。
     */
    void stop();

    /**
     * 销毁线程：将停止标志置为 false（表示线程应退出），然后唤醒线程并 join。
     */
    void destory();

    /**
     * 派生类必须实现此方法；后台线程在被唤醒后会调用一次该方法执行工作逻辑。
     * 该方法不应该在基类或其他线程析构期间被调用（否则可能发生纯虚调用问题）。
     */
    virtual void run() = 0;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;   // PIMPL：隐藏实现细节（condition_variable、thread、atomic）
};
}

#endif // THREAD_H