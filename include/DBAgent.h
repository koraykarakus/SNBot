#pragma once

#include <mysql/mysql.h>
#include <string>
#include "SimpleIni.h"

class DBAgent
{
private:
    MYSQL* m_pConn;
    std::string m_strDBUser;
    std::string m_strDBPass;
    std::string m_strDBHost;
    std::string m_strDBName;
    std::string m_strDBPrefix;
public:
    DBAgent();
    ~DBAgent();
    void Init();

    bool Connect();
    void Disconnect();
    bool LoadBots();

    MYSQL* GetConnection() const { return m_pConn; }
};