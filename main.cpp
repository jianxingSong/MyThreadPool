#include <iostream>
#include "thread"
#include "vector"
#include "queue"
#include "functional"
#include "utility"

using namespace std;

class ThreadPool{
public:
    ThreadPool(int threadNum){
        for(int i = 0;i < threadNum;i++){
            m_workers.emplace_back([this](){
                while (1){
                    unique_lock<mutex> lock(m_mtx);
                    m_cond.wait(lock, [this](){
                        return m_tasks.size() > 0 || finish;
                    });

                    if(finish && m_tasks.size() == 0)
                        break;

                    function<void()> task(move(m_tasks.front()));
                    m_tasks.pop();
                    lock.unlock();
                    task();
                }
            });
        }
    }

    template<typename F, typename... Args>
    void enqueue(F f, Args... args){
        function<void()> task = bind(forward<F>(f), forward<Args>(args)...);
        {
            unique_lock<mutex> lock(m_mtx);
            m_cond.wait(lock, [this](){
                return !finish;
            });
            m_tasks.push(task);
        }
        m_cond.notify_one();
    }


    //负责将finish置为true，然后等待线程完成
    ~ ThreadPool(){
        {
            unique_lock<mutex> lock(m_mtx);
            finish = true;
        }

        m_cond.notify_all();
        for(auto& t : m_workers)
            t.join();
    }



private:
    vector<thread> m_workers;
    queue<function<void()>> m_tasks; //std::function<void()>具体地表示一个不返回值并且不接受任何参数的函数对象。
    mutex m_mtx;
    condition_variable m_cond;
    bool finish;
};




int main() {
    ThreadPool threadPool(5);
    for(int i = 1;i < 10;i++){
        threadPool.enqueue([](int i){
            cout << "worker works for " << i << endl;
            this_thread::sleep_for(chrono::seconds(1));
            cout << "worker finished " << i << endl;
        }, i);
    }



    return 0;
}
