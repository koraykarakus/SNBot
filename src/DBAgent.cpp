#include "DBAgent.h"
#include "logger.h"
#include <filesystem>

DBAgent::DBAgent()
    : m_pConn(nullptr)
    , m_strDBUser()
    , m_strDBPass()
    , m_strDBHost()
    , m_strDBName()
    , m_strDBPrefix()
{
    Init();
}

DBAgent::~DBAgent()
{
    Disconnect();
}

void DBAgent::Init() 
{
    // 1. EXE ile aynı klasördeki settings.ini dosyasının yolunu buluyoruz
    // std::filesystem hem Windows hem Linux platformlarında sorunsuz çalışır.
    std::filesystem::path iniPath = std::filesystem::current_path() / "settings.ini";
    std::string iniPathStr = iniPath.string();

    CSimpleIniA ini;
    ini.SetUnicode(); // Türkçe veya özel karakter desteği için

    // 2. INI dosyasını yüklemeyi dene
    SI_Error rc = ini.LoadFile(iniPathStr.c_str());

    // create settings.ini if it is not found
    if (rc < 0) 
    {
        Logger::Error("settings.ini not found check folder and fill required info !\n");

        // Varsayılan değerleri belleğe yaz
        ini.SetValue("Database", "Host", "localhost");
        ini.SetValue("Database", "User", "username");
        ini.SetValue("Database", "Password", "password");
        ini.SetValue("Database", "DBName", "steemnova");
        ini.SetValue("Database", "Prefix", "uni1_"); 

        // Değerleri fiziksel olaraksettings.ini dosyasına kaydet
        rc = ini.SaveFile(iniPathStr.c_str());
        if (rc < 0) 
        {
            Logger::Error("error : settings.ini cannot be saved !\n");
        }
    }

    // 3. Değerleri INI dosyasından (veya yeni oluşturulan veriden) sınıfın üye değişkenlerine oku
    // GetValue fonksiyonunun 3. parametresi, eğer INI'de o key yoksa dönecek olan "fallback" değerdir.
    m_strDBHost = ini.GetValue("Database", "Host");
    m_strDBUser = ini.GetValue("Database", "User");
    m_strDBPass = ini.GetValue("Database", "Password");
    m_strDBName = ini.GetValue("Database", "DBName");
    m_strDBPrefix = ini.GetValue("Database", "Prefix");

    Logger::Info("[DBAgent] settings read from settings.ini Host: {}", m_strDBHost);
}

bool DBAgent::Connect()
{
    m_pConn = mysql_init(nullptr);

    if (m_pConn == nullptr)
    {
        Logger::Error("mysql_init failed");
        return false;
    }

    // close ssl
    int ssl_verify = 0; 
    mysql_optionsv(m_pConn, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &ssl_verify);
    // ----------------------------------------

    if (!mysql_real_connect(
        m_pConn,
        m_strDBHost.c_str(),
        m_strDBUser.c_str(),
        m_strDBPass.c_str(),
        m_strDBName.c_str(),
        3306,
        nullptr,
        0))
    {
        Logger::Error("MySQL Connection Failed. Error: {} ({})", mysql_error(m_pConn), mysql_errno(m_pConn));

        // Şimdi güvenle kapatabiliriz
        mysql_close(m_pConn);
        m_pConn = nullptr;
        return false;
    }

    return true;
}

void DBAgent::Disconnect()
{
    if (m_pConn != nullptr)
    {
        mysql_close(m_pConn);
        m_pConn = nullptr;
    }
}

bool DBAgent::LoadBots() 
{
    // İleride INI'den std::string olarak okuyacaksın
    std::string strPrefix = "";
    std::string strQuery = "SELECT * FROM `" + strPrefix + "users` WHERE is_bot = 1";
    // 1. Sorguyu sunucuya gönderiyoruz
    if (mysql_query(m_pConn, strQuery.c_str()))
    {
        Logger::Error("Query Error: {}", mysql_error(m_pConn));
        return false;
    }

    // 2. Dönen sonuç kümesini (Result Set) belleğe alıyoruz
    MYSQL_RES* result = mysql_store_result(m_pConn);

    if (!result)
    {
        // Sorgu başarılı ama geriye bir tablo dönmediyse (örn: UPDATE, INSERT) 
        // ya da ciddi bir hata oluştuysa buraya düşer.
        if (mysql_field_count(m_pConn) > 0)
        {
            Logger::Error("Retrieve result error: {}", mysql_error(m_pConn));
        }
        return false;
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
    return true;
}