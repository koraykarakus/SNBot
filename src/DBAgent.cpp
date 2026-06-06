#include "DBAgent.h"
#include "logger.h"

DBAgent::DBAgent()
{
    m_conn = nullptr;
}

DBAgent::~DBAgent()
{
    Disconnect();
}

bool DBAgent::Connect()
{
    m_conn = mysql_init(nullptr);

    if (!m_conn) 
    {
        Logger::Error("mysql_init failed");
        return false;
    }

    // --- MARIADB İÇİN SSL'İ KAPATMA ADIMI ---
    // my_bool veri tipi mysql.h içinde tanımlıdır (aslında char veya bool'dur)
    int ssl_verify = 0; // MARIADB_SSL_ALLOWED_NO anlamına gelir
    mysql_optionsv(m_conn, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &ssl_verify);
    // ----------------------------------------

    if (!mysql_real_connect(
        m_conn,
        "localhost",
        "koray",
        "14531071",
        "steemnova",
        3306,
        nullptr,
        0))
    {
        Logger::Error("MySQL Connection Failed. Error: {} ({})", mysql_error(m_conn), mysql_errno(m_conn));

        // Şimdi güvenle kapatabiliriz
        mysql_close(m_conn);
        m_conn = nullptr;
        return false;
    }

    return true;
}

void DBAgent::Disconnect()
{
    if (m_conn)
    {
        mysql_close(m_conn);
        m_conn = nullptr;
    }
}

void DBAgent::LoadBots() 
{
    const char* query = "SELECT * FROM `users` WHERE is_bot = 1";

    // 1. Sorguyu sunucuya gönderiyoruz
    if (mysql_query(m_conn, query))
    {
        Logger::Error("Query Error: {}", mysql_error(m_conn));
        return;
    }

    // 2. Dönen sonuç kümesini (Result Set) belleğe alıyoruz
    MYSQL_RES* result = mysql_store_result(m_conn);

    if (!result)
    {
        // Sorgu başarılı ama geriye bir tablo dönmediyse (örn: UPDATE, INSERT) 
        // ya da ciddi bir hata oluştuysa buraya düşer.
        if (mysql_field_count(m_conn) > 0)
        {
            Logger::Error("Retrieve result error: {}", mysql_error(m_conn));
        }
        return;
    }

    // Tabloda kaç adet bot satırı döndüğünü loglayalım
    uint64_t rowCount = mysql_num_rows(result);
    Logger::Info("Found {} bots in database.", rowCount);

    // 3. Satırları tek tek döngüyle okuyoruz
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
    {
        // NOT: Veritabanındaki kolon sırasına göre index belirtmelisin.
        // Örn: id=0, username=1, password=2, is_bot=3 gibi...
        // row[X] her zaman 'const char*' (string) döner. Sayısal değerleri atoi/atof ile çevirmelisin.

        const char* botId = row[0];       // 1. kolon (genelde id olur)
        const char* botName = row[1];     // 2. kolon (genelde username olur)

        Logger::Info("Bot Loaded -> ID: {}, Name: {}", botId, botName);

        // Burada bot nesnelerini oluşturup bir vektöre (std::vector) atabilirsin.
    }

    // 4. KRİTİK ADIM: Belleği temizliyoruz. 
    // Bunu yapmazsan her sorgu çağrıldığında RAM şişer (Memory Leak).
    mysql_free_result(result);
}