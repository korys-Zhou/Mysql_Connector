#include <stdio.h>
#include <iostream>
#include <vector>
#include "ConnPool.h"

bool init() {
    MysqlConn conn;
    conn.connect("root", "Password1!", "dbtest", "127.0.0.1");
    std::string sql = "truncate person";
    if (!conn.update(sql))
        std::cout << "initialization failed.\n";
    return true;
}

void withoutPool(int begin, int end) {
    for (int i = begin; i < end; ++i) {
        MysqlConn conn;
        conn.connect("root", "Password1!", "dbtest", "127.0.0.1");
        char sql[1024] = {0};
        sprintf(sql, "insert into person values(%d,'foo',0,0.0)", i);
        conn.update(sql);
    }
}

void withPool(ConnPool* pool, int begin, int end) {
    for (int i = begin; i < end; ++i) {
        std::shared_ptr<MysqlConn> conn = pool->getConn();
        char sql[1024] = {0};
        sprintf(sql, "insert into person values(%d,'foo',0,0.0)", i);
        conn->update(sql);
    }
}

void singleThreadWithoutPool() {
    if (!init())
        return;
    std::chrono::steady_clock::time_point begin =
        std::chrono::steady_clock::now();

    withoutPool(0, 1000);

    std::chrono::steady_clock::time_point end =
        std::chrono::steady_clock::now();
    auto dur = end - begin;
    std::cout << "Single thread without pool consume " << dur.count() / 1000000
              << " ms.\n";
}

void singleThreadWithPool() {
    if (!init())
        return;
    ConnPool* CP = ConnPool::getInstance();
    std::chrono::steady_clock::time_point begin =
        std::chrono::steady_clock::now();

    withPool(CP, 0, 1000);

    std::chrono::steady_clock::time_point end =
        std::chrono::steady_clock::now();
    auto dur = end - begin;
    std::cout << "Single thread with pool consume " << dur.count() / 1000000
              << " ms.\n";
}

void multiThreadWithoutPool() {
    if (!init())
        return;
    MysqlConn precon;
    precon.connect("root", "Password1!", "dbtest", "127.0.0.1");
    std::chrono::steady_clock::time_point begin =
        std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.push_back(std::thread(withoutPool, i * 200, (i + 1) * 200));
    }
    for (auto& i : threads)
        i.join();

    std::chrono::steady_clock::time_point end =
        std::chrono::steady_clock::now();
    auto dur = end - begin;
    std::cout << "Multi thread without pool consume " << dur.count() / 1000000
              << " ms.\n";
}

void multiThreadWithPool() {
    if (!init())
        return;
    ConnPool* CP = ConnPool::getInstance();
    std::chrono::steady_clock::time_point begin =
        std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.push_back(std::thread(withPool, CP, i * 200, (i + 1) * 200));
    }
    for (auto& i : threads)
        i.join();

    std::chrono::steady_clock::time_point end =
        std::chrono::steady_clock::now();
    auto dur = end - begin;
    std::cout << "Multi thread with pool consume " << dur.count() / 1000000
              << " ms.\n";
}

int main() {
    singleThreadWithoutPool();
    singleThreadWithPool();
    multiThreadWithoutPool();
    multiThreadWithPool();
    return 0;
}