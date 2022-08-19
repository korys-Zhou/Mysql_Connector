#include "ConnPool.h"

ConnPool::ConnPool() {
    if (!readPropertyJson()) {
        return;
    }

    for (int i = 0; i < m_minsize; ++i) {
        createConn();
    }

    std::thread th_producer(&ConnPool::produceConn, this),
        th_recycler(&ConnPool::recycleConn, this);
    th_producer.detach();
    th_recycler.detach();
}

ConnPool::~ConnPool() {
    while (!m_conns.empty()) {
        MysqlConn* conn = m_conns.front();
        m_conns.pop();
        delete conn;
    }
}

bool ConnPool::readPropertyJson() {
    std::ifstream ifs("connpool_property.json");
    Json::Reader rd;
    Json::Value root;
    rd.parse(ifs, root);
    if (root.isObject()) {
        m_conninfo.ip = root["ip"].asString();
        m_conninfo.port = root["port"].asInt();
        m_conninfo.user = root["user"].asString();
        m_conninfo.passwd = root["password"].asString();
        m_conninfo.dbname = root["dbname"].asString();
        m_minsize = root["minsize"].asInt();
        m_maxsize = root["maxsize"].asInt();
        m_max_idle_time = root["maxidletime"].asInt();
        m_timeout = root["timeout"].asInt();
        return true;
    }
    return false;
}

void ConnPool::createConn() {
    MysqlConn* conn = new MysqlConn();
    conn->connect(m_conninfo);
    conn->resetAliveTime();
    m_conns.push(conn);
}

void ConnPool::produceConn() {
    while (true) {
        std::unique_lock<std::mutex> lk(m_mtx);
        while (m_conns.size() >= m_minsize) {
            m_cond_produce.wait(lk);
        }
        createConn();
        m_cond_consume.notify_all();
    }
}

void ConnPool::recycleConn() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::lock_guard<std::mutex> lg(m_mtx);
        while (m_conns.size() > m_minsize) {
            MysqlConn* conn = m_conns.front();
            if (conn->getAliveTime() >= m_max_idle_time) {
                m_conns.pop();
                delete conn;
            } else
                break;
        }
    }
}

std::shared_ptr<MysqlConn> ConnPool::getConn() {
    std::unique_lock<std::mutex> lk(m_mtx);
    while (m_conns.empty()) {
        if (std::cv_status::timeout ==
            m_cond_consume.wait_for(lk, std::chrono::milliseconds(m_timeout))) {
            if (m_conns.empty()) {
                m_cond_produce.notify_all();
            }
        }
    }
    std::shared_ptr<MysqlConn> ret(m_conns.front(), [this](MysqlConn* conn) {
        std::lock_guard<std::mutex> lg(m_mtx);
        m_conns.push(conn);
        conn->resetAliveTime();
    });
    m_conns.pop();
    m_cond_produce.notify_all();
    return ret;
}