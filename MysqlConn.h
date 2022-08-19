#ifndef MYSQL_CONNECTOR_MYSQLCONN_H_
#define MYSQL_CONNECTOR_MYSQLCONN_H_

#include <inttypes.h>
#include <mysql/mysql.h>
#include <chrono>
#include <string>

struct ConnInfo {
    std::string user;
    std::string passwd;
    std::string dbname;
    std::string ip;
    uint16_t port;

    ConnInfo() {}

    ConnInfo(std::string user,
             std::string passwd,
             std::string dbname,
             std::string ip,
             uint16_t port = 3306)
        : user(user), passwd(passwd), dbname(dbname), ip(ip), port(port) {}
};

class MysqlConn {
   public:
    MysqlConn();
    ~MysqlConn();

    bool connect(std::string user,
                 std::string passwd,
                 std::string dbname,
                 std::string ip,
                 uint16_t port = 3306);

    bool connect(ConnInfo info);

    bool update(std::string sql);

    bool query(std::string sql);

    bool next();

    std::string getValue(int index);

    bool transaction();

    bool commit();

    bool rollback();

    void resetAliveTime();

    long long getAliveTime();

   private:
    void freeResult();

   private:
    MYSQL* m_conn;
    MYSQL_RES* m_res;
    MYSQL_ROW m_row;
    std::chrono::steady_clock::time_point m_alivetime;
};

#endif