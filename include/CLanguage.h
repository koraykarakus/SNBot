#pragma once
#include <string>
#include <unordered_map>

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
public:
	lang lang_;
	std::unordered_map<std::string, std::string> strings_;

private:
	void Init();
	void CreateDefaultLangFile();
	bool LoadLangFile();
	std::string GetLangTail();
public:
	CLanguage();
	~CLanguage();
	inline std::unordered_map<std::string, std::string>& GetLangStrings()
	{
		return strings_;
	}
};