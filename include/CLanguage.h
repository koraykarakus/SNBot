#pragma once
#include <string>
#include <unordered_map>
#include <array>

enum class lang
{
	de,
	en,
	es,
	fr,
	pl,
	pt,
	ru,
	tr
};

class CLanguage
{
private:
	const std::array<std::string,8> s_lang_arr {
		"de", "en", "es", "fr", "pl", "pt", "tr", "ru"};
public:
	lang lang_;
	std::unordered_map<std::string, std::string> strings_;

private:
	void Init();
	void CreateDefaultLangFile();

public:
	CLanguage();
	~CLanguage();
	bool ExistsLang(std::string& lang_key);
	bool LoadLangFile(std::string lang_key = "en");
	inline std::unordered_map<std::string, std::string>* GetLangStrings()
	{
		return &strings_;
	}
};