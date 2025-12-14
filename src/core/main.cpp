#include "workqueue.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

using namespace lmc;
using namespace std;

int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

/**
 * 演示任务：文件处理模拟
 * filename 文件名
 * string 处理结果
 */
string process_file(const string& filename) {
    this_thread::sleep_for(chrono::milliseconds(200));
    return "文件 " + filename + " 处理完成";
}

/**
 * 显示程序使用说明
 */
void show_usage() {
    cout << "\n=== WorkQueue 线程池演示程序 ===" << endl;
    cout << "1. 计算斐波那契数列" << endl;
    cout << "2. 文件处理模拟" << endl;
    cout << "3. 批量任务测试" << endl;
    cout << "4. 运行完整测试套件" << endl;
    cout << "0. 退出程序" << endl;
    cout << "请选择操作 (0-4): ";
}

/**
 * 执行斐波那契计算演示
 */
void demo_fibonacci() {
    cout << "\n--- 斐波那契计算演示 ---" << endl;
    
    WorkQueue queue(MutexType::Mutex);
    
    cout << "提交计算任务..." << endl;
    auto task1 = queue.addTask(fibonacci, 10);
    auto task2 = queue.addTask(fibonacci, 15);
    auto task3 = queue.addTask(fibonacci, 20);
    
    cout << "等待计算结果..." << endl;
    cout << "fib(10) = " << task1.get() << endl;
    cout << "fib(15) = " << task2.get() << endl;
    cout << "fib(20) = " << task3.get() << endl;
}

/**
 * 执行文件处理演示
 */
void demo_file_processing() {
    cout << "\n--- 文件处理演示 ---" << endl;
    
    WorkQueue queue(MutexType::Spin);
    
    vector<string> files = {"document.txt", "image.jpg", "data.csv", "report.pdf"};
    vector<future<string>> results;
    
    for (const auto& file : files) {
        results.push_back(queue.addTask(process_file, file));
    }
    
    cout << "文件处理中..." << endl;
    for (size_t i = 0; i < results.size(); ++i) {
        cout << results[i].get() << endl;
    }
}

/**
 *执行批量任务演示
 */
void demo_batch_tasks() {
    cout << "\n--- 批量任务演示 ---" << endl;
    
    WorkQueue queue(MutexType::Mutex);
    auto start_time = chrono::high_resolution_clock::now();
    
    vector<future<int>> tasks;
    for (int i = 0; i < 8; ++i) {
        tasks.push_back(queue.addTask([](int x) {
            this_thread::sleep_for(chrono::milliseconds(100));
            return x * x;
        }, i));
    }
    
    int total = 0;
    for (size_t i = 0; i < tasks.size(); ++i) {
        int result = tasks[i].get();
        total += result;
        cout << "任务" << i << "结果: " << result << endl;
    }
    
    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
    
    cout << "总和: " << total << endl;
    cout << "总耗时: " << duration.count() << "ms" << endl;
}

/**
 *程序主入口
 * 
 *提供交互式菜单，让用户选择不同的演示功能。
 * 
 * int 程序退出码
 */
int main() {
    int choice = 0;
    
    do {
        show_usage();
        cin >> choice;
        
        switch (choice) {
            case 1:
                demo_fibonacci();
                break;
            case 2:
                demo_file_processing();
                break;
            case 3:
                demo_batch_tasks();
                break;
            case 4:
                // 运行测试套件
                cout << "\n运行完整测试套件..." << endl;
                // 这里可以调用测试函数
                break;
            case 0:
                cout << "程序退出" << endl;
                break;
            default:
                cout << "无效选择，请重新输入" << endl;
                break;
        }
        
        if (choice != 0) {
            cout << "\n按回车键继续...";
            cin.ignore();
            cin.get();
        }
        
    } while (choice != 0);
    
    return 0;
}