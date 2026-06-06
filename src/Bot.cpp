#include "Bot.h"
#include "logger.h"
#include <iostream>

// constructor
Bot::Bot(GameInfo* pGameInfo, const std::string& strEmail, const std::string& strPassword)
    : m_pGameInfo(pGameInfo)
    , m_strEmail(strEmail)
    , m_strPassword(strPassword)
    , m_curl(nullptr)
{
}

// destructor
Bot::~Bot()
{
    if (m_curl != nullptr) 
    {
        curl_easy_cleanup(m_curl);
        Logger::Info("Bot oturumu kapatildi");
    }
}

size_t Bot::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t totalSize = size * nmemb;
    std::string* responseBuffer = static_cast<std::string*>(userp);
    responseBuffer->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

bool Bot::Init()
{
    m_curl = curl_easy_init();
    if (m_curl == nullptr) 
    {
        Logger::Error("CURL init hatasi!");
        return false;
    }

    // KRİTİK: Çerez (Cookie) yönetimini açıyoruz. 
    // Tırnak içini boş bırakırsak çerezler RAM üzerinde (hafızada) tutulur, dosyaya yazılmaz.
    curl_easy_setopt(m_curl, CURLOPT_COOKIEFILE, "");

    // Sunucudan gelen yanıtları okumak için callback fonksiyonumuzu bağlıyoruz
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteCallback);

    // İsteğe standart bir tarayıcı süsü vermek için (Gerekirse)
    curl_easy_setopt(m_curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");

    return true;
}

bool Bot::Register(const std::string& strRegisterUrl,
    const std::string& strPostData) 
{
    if (m_curl == nullptr) 
    {
        return false;
    }

    std::string responseBuffer;

    // Hedef URL ve POST verilerini set ediyoruz
    curl_easy_setopt(m_curl, CURLOPT_URL, strRegisterUrl.c_str());
    curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, strPostData.c_str());
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &responseBuffer);

    std::string strMsg;
    Logger::Info("Request register has been sent.");
    CURLcode res = curl_easy_perform(m_curl);

    if (res != CURLE_OK) 
    {
        Logger::Error("HTTP error : (Register) failed");
        return false;
    }

    // --- DOĞRULAMA (KONTROL) ---
    // F12 ile baktığında giriş başarılı olunca HTML içinde kesin çıkan bir kelime seç.
    // Örneğin oyun Türkçe ise "Genel Durum", "Gezegen", "Hammadde" veya "Cikis" butonu gibi.
    if (responseBuffer.find("Cikis") != std::string::npos 
        || responseBuffer.find("Overview") != std::string::npos) 
    {
        Logger::Info("register success.");
        return true;
    }

    Logger::Error("register failed");
    return false;
}

bool Bot::Login()
{
    if (m_curl == nullptr
        || m_pGameInfo == nullptr)
    {
        return false;
    }

    std::string responseBuffer;

    std::string strLoginData =
        "email="+ m_strEmail +"&password=" + m_strPassword +
        "&g_recaptcha_response=0&remember_me=0&universe=1" +
        "&remembered_token_validator=''&remembered_token_selector=''";

    // Hedef URL ve POST verilerini set ediyoruz
    curl_easy_setopt(m_curl, CURLOPT_URL, m_pGameInfo->GetLoginPageUrl().c_str());
    curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, strLoginData.c_str());
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &responseBuffer);
    Logger::Info("Request login has been sent.");
    CURLcode res = curl_easy_perform(m_curl);

    if (res != CURLE_OK)
    {
        Logger::Error("HTTP error : (login) failed");
        return false;
    }

    // --- DOĞRULAMA (KONTROL) ---
    // F12 ile baktığında giriş başarılı olunca HTML içinde kesin çıkan bir kelime seç.
    // Örneğin oyun Türkçe ise "Genel Durum", "Gezegen", "Hammadde" veya "Cikis" butonu gibi.
    if (responseBuffer.find("redirect") != std::string::npos
        || responseBuffer.find("Success") != std::string::npos)
    {
        return true;
    }

    Logger::Error("login failed");
    return false;
}

bool Bot::SendBuilding(int iID)
{
    if (m_curl == nullptr 
        || m_pGameInfo == nullptr)
    {
        return false;
    }

    std::string strData = "cmd=insert&building=" + std::to_string(iID);
    std::string responseBuffer;

    // 1. ADIM: Sunucuya bunun bir form verisi olduğunu açıkça söylemeliyiz
    struct curl_slist* headers = nullptr;
    CurlStartPost(headers);
    // Ayarları yapıyoruz
    curl_easy_setopt(m_curl, CURLOPT_URL, m_pGameInfo->GetBuildingsPageUrl().c_str());
    curl_easy_setopt(m_curl, CURLOPT_POST, 1L);
    curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, strData.c_str());
    curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(strData.length()));
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &responseBuffer);

    Logger::Info("Bina insa istegi gonderiliyor...");
    CURLcode res = curl_easy_perform(m_curl);

    // 2. ADIM (KRİTİK): İşimiz bittiği an curl handle'ını bir sonraki 
    // istek için temizliyoruz!
    // Burayı yapmazsak handle kilitlenir veya sonraki istekleri bozar.
    CurlCleanUp(headers);

    if (res == CURLE_OK)
    {
        Logger::Info("Bina istegi sunucuya ulasti.");

        // Hâlâ basmıyorsa sunucunun ne dediğini görmek için burayı geçici olarak açabilirsin:
        // if (responseBuffer.find("Yetersiz") != std::string::npos) Logger::Warn("Maden yetersiz olabilir!");

        return true;
    }

    Logger::Error("Bina istegi gonderilemedi: ");
    return false;
}

void Bot::CurlCleanUp(struct curl_slist*& pHeaders)
{
    if (m_curl == nullptr) return;

    // 1. Curl handle üzerindeki durumları ve verileri sıfırla
    curl_easy_setopt(m_curl, CURLOPT_POST, 0L);
    curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, nullptr);
    curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, 0L);
    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, nullptr);

    // 2. Dinamik olarak ayrılan header listesini temizle
    if (pHeaders != nullptr)
    {
        curl_slist_free_all(pHeaders);
        pHeaders = nullptr; // Dangling pointer olmaması için güvenliğe alıyoruz
    }
}

void Bot::CurlStartPost(struct curl_slist*& pHeaders)
{
    if (m_curl == nullptr) return;

    // Tedbir amaçlı, dışarıdan gelen pointer'ı temiz bir başlangıca zorluyoruz
    pHeaders = nullptr;

    // Header listesini oluşturuyoruz
    pHeaders = curl_slist_append(pHeaders, "Content-Type: application/x-www-form-urlencoded");

    // Eğer curl_slist_append başarısız olursa, pHeaders zaten nullptr kalır.
    // libcurl nullptr'ı temizlik olarak kabul ettiği için çökme riski sıfırdır.
    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, pHeaders);
    curl_easy_setopt(m_curl, CURLOPT_POST, 1L);
}

