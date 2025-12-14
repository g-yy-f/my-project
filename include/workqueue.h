#ifndef WORKQUEUE_H_
#define WORKQUEUE_H_

#include "lthread.h"
#include "src/util/spinmutex.hpp"

#include <future>
#include <iostream>
#include <functional>
#include <queue>
#include <mutex>

using namespace std;

namespace lmc {

/**
 * MutexType - 支持的互斥策略
 * - None: 不做任何互斥（由使用者保证安全）
 * - Spin: 使用自旋锁（SpinMutex）
 * - Mutex: 使用 std::mutex
 */
enum class MutexType: unsigned char {
    None,
    Spin,
    Mutex,
};

/**
 * SMutex - 可切换的互斥封装
 *
 * 根据设置的 MutexType 决定 lock()/unlock() 的实现：
 * - 在 MutexType::Mutex 时使用 std::mutex，适合线程间阻塞等待场景
 * - 在 MutexType::Spin 时使用自旋锁，适合短临界区、频繁争用且对延迟敏感的场景
 * - 在 MutexType::None 时不进行互斥（由调用者保证并发安全）
 */
class SMutex {
public:
    SMutex() = default;
    void lock();
    void unlock();
    void setMutexType(MutexType m);

private:
    SpinMutex mSpinMutex;
    mutex mMutex;
    MutexType mMutexType;
};

/**
 * WorkQueue - 基于 Thread 的任务队列实现
 *
 * 功能：
 * - 提供模板方法 addTask，将任意可调用对象封装成任务并返回 std::future
 * - 任务被放入内部队列，并唤醒后台线程（基类 Thread）去执行任务
 * - 支持通过 MutexType 选择不同的互斥策略
 *
 * 设计要点：
 * - addTask 将函数与参数绑定到 packaged_task，使用 shared_ptr 管理生命周期，
 *   将任务包装为无参的 std::function<void()> 放入队列
 * - run() 在后台线程中被调用一次，负责从队列取出并执行一个任务
 * - 析构函数在清空队列后会调用 destory() 停止并 join 基类线程，避免纯虚函数在析构时被调用
 */
class WorkQueue final : public Thread {
public:
    WorkQueue(MutexType m);
    ~WorkQueue();

    WorkQueue(const WorkQueue &) = delete;
    WorkQueue(WorkQueue &&) = delete;
    WorkQueue &operator=(WorkQueue &&) = delete;

    /**
     * 添加任务
     * 第一个参数为任务函数（可为 lambda、函数指针、可调用对象），后续参数为该函数的参数
     * 返回 std::future<ReturnType>，用于获取任务最终返回值或等待任务完成
     *
     * 逻辑说明：
     * - 使用 std::packaged_task 将可调用对象包装为可获取 future 的形式
     * - 将任务推入队列后调用 start() 唤醒后台线程执行 run()
     * - 使用共享指针管理 packaged_task 的生命周期，确保队列中的可调用对象在执行期间有效
     */
    template <typename F, typename ...Args>
    auto addTask(F &&f, Args &&...args) throw() ->
    future<typename result_of<F(Args...)>::type> {
        using returnType = typename result_of<F(Args...)>::type;
        auto task = make_shared<packaged_task<returnType()>>(bind(
                                forward<F>(f), forward<Args>(args)...));
        future<returnType> returnRes = task.get()->get_future();

        // 对队列进行互斥保护（由 SMutex 根据类型决定具体实现）
        mutex.lock();
        workqueue.emplace([task]{(*task)();});
        mutex.unlock();

        // 唤醒后台线程执行任务
        start();
        return returnRes;
    }

    /**
     * 显式停止工作队列的唤醒（不等同于销毁线程）
     */
    void stopWorkQueue();

protected:
    /**
     * run() - 后台线程每次被唤醒后调用该函数
     *
     * 实现逻辑：
     * - 加锁检查队列是否为空；若为空则直接返回（不消费任务）
     * - 否则弹出队首任务并在解锁后执行，避免长时间持锁阻塞其他提交者
     */
    void run() override;

private:
    queue<function<void()>> workqueue; // 任务队列
    SMutex mutex;                      // 可切换的互斥体
};
}

#endif