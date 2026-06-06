#pragma once

#include <mysql/mysql.h>
#include <string>


class CDatabase
{
private:
    MYSQL* m_pConn;
    std::string m_strDBUser;
    std::string m_strDBPass;
    std::string m_strDBHost;
    std::string m_strDBName;
    std::string m_strDBPrefix;
public:
    CDatabase();
    ~CDatabase();
    void Init();

    bool Connect();
    void Disconnect();
    bool LoadBots();
    bool LoadVars();
    bool UpdateBots();

    MYSQL* GetConnection() const { return m_pConn; }
};

extern CDatabase g_Database;
