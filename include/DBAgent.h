#pragma once

#include <mysql/mysql.h>

class DBAgent
{
private:
    MYSQL* m_conn;

public:
    DBAgent();
    ~DBAgent();

    bool Connect();
    void Disconnect();
    void LoadBots();

    MYSQL* GetConnection() const { return m_conn; }
};