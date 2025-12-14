#ifndef SPINMUTEX_HPP_
#define SPINMUTEX_HPP_

#include <atomic>

namespace lmc {

/**
 * SpinMutex - 基于原子标志的自旋锁（轻量级）
 *
 * 特点：
 * - 适合短临界区并且对延迟敏感的场景
 * - 使用 compare_exchange_weak 原子操作进行尝试获取锁，失败则不断重试（繁忙等待）
 * - unlock() 将标志置为 false，允许其他线程获取锁
 */
class SpinMutex {
public:
    SpinMutex() : flag_(false) {}

    inline void lock() {
        bool expect = false;
        // compare_exchange_weak 在失败时会修改 expect，所以每次循环需要重置 expect
        while (!flag_.compare_exchange_weak(expect, true)) {
            expect = false;
        }
    }

    inline void unlock() {
        flag_.store(false);
    }

private:
    std::atomic<bool> flag_;
};
}

#endif