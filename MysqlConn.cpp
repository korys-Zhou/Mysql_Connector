#include "MysqlConn.h"

MysqlConn::MysqlConn()
    : m_conn(mysql_init(nullptr)), m_res(nullptr), m_row(nullptr) {
    mysql_set_character_set(m_conn, "utf8");
}

MysqlConn::~MysqlConn() {
    freeResult();
    if (m_conn)
        mysql_close(m_conn);
}

bool MysqlConn::connect(std::string user,
                        std::string passwd,
                        std::string dbname,
                        std::string ip,
                        uint16_t port) {
    MYSQL* p =
        mysql_real_connect(m_conn, ip.c_str(), user.c_str(), passwd.c_str(),
                           dbname.c_str(), port, nullptr, 0);
    return p;
}

bool MysqlConn::connect(ConnInfo info) {
    MYSQL* p = mysql_real_connect(m_conn, info.ip.c_str(), info.user.c_str(),
                                  info.passwd.c_str(), info.dbname.c_str(),
                                  info.port, nullptr, 0);
    return p;
}

bool MysqlConn::update(std::string sql) {
    if (mysql_query(m_conn, sql.c_str()))
        return false;
    return true;
}

bool MysqlConn::query(std::string sql) {
    freeResult();
    if (mysql_query(m_conn, sql.c_str()))
        return false;
    m_res = mysql_store_result(m_conn);
    return true;
}

bool MysqlConn::next() {
    if (!m_res)
        return false;
    m_row = mysql_fetch_row(m_res);
    return m_row;
}

std::string MysqlConn::getValue(int index) {
    int nCol = mysql_num_fields(m_res);
    if (index >= nCol || index < 0)
        return "";
    char* val = m_row[index];
    unsigned long len = mysql_fetch_lengths(m_res)[index];
    return std::string(val, len);
}

bool MysqlConn::transaction() {
    return mysql_autocommit(m_conn, false);
}

bool MysqlConn::commit() {
    return mysql_commit(m_conn);
}

bool MysqlConn::rollback() {
    return mysql_rollback(m_conn);
}

void MysqlConn::resetAliveTime() {
    m_alivetime = std::chrono::steady_clock::now();
}

long long MysqlConn::getAliveTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - m_alivetime)
        .count();
}

void MysqlConn::freeResult() {
    if (m_res) {
        mysql_free_result(m_res);
        m_res = nullptr;
    }
}