#ifndef MYSQL_CONNECTOR_CONNPOOL_H_
#define MYSQL_CONNECTOR_CONNPOOL_H_

#include <json/json.h>
#include <condition_variable>
#include <fstream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include "MysqlConn.h"

class ConnPool {
   private:
    ConnPool();

   public:
    ConnPool(const ConnPool&) = delete;
    ConnPool& operator=(const ConnPool&) = delete;
    ~ConnPool();

   private:
    bool readPropertyJson();
    void createConn();
    void produceConn();
    void recycleConn();

   public:
    std::shared_ptr<MysqlConn> getConn();

   public:
    static ConnPool* getInstance() {
        static ConnPool CP{};
        return &CP;
    }

   private:
    ConnInfo m_conninfo;
    int m_minsize, m_maxsize, m_timeout, m_max_idle_time;
    std::queue<MysqlConn*> m_conns;
    std::mutex m_mtx;
    std::condition_variable m_cond_produce, m_cond_consume;
};

#endif