#pragma once
#include <string>
#include <map>
#include "CLogger.h"

enum class type
{
	type_numeric,
	type_double,
	type_string,
	type_array
};

struct php_val
{
	type value_type;

	int numeric_val = 0;
	std::string string_val;

	std::map<int, php_val> array_val;

	// operators..
	php_val& operator[](int index)
	{
		return array_val[index];
	}

	const php_val& operator[](int index) const
	{
		return array_val.at(index);
	}

};

class CPhpHelper
{
private:
	// reading position
	size_t pos_;

	// pointer to input string
	std::string input_;

public:
	CPhpHelper()
		: pos_(0)
		, input_()
	{}
	~CPhpHelper();
	
	void Unserialize(const std::string& str,
		std::map<int, php_val>& result)
	{
		result.clear();

		input_ = str;
		pos_ = 0;

		php_val root = ParseValue();

		if (root.value_type == type::type_array)
		{
			result = root.array_val;
		}
	}

private:
	inline char Peek()
	{
		return input_[pos_];
	}

	inline char Get()
	{
		return input_[pos_++];
	}

	inline void Expect(char c)
	{
		if (Get() != c)
		{
			CLogger::Error("parse error check Expect !");
		}
	}

	std::string ReadUntil(char end_char)
	{
		size_t start = pos_;

		while (pos_ < input_.length() 
			&& input_[pos_] != end_char)
		{
			pos_++;
		}

		std::string result = input_.substr(start, pos_ - start);

		Expect(end_char);

		return result;
	}

	php_val ParseValue()
	{
		char type = Get();

		switch (type)
		{
			case 'i':
			case 'd':
				return ParseNumeric();

			case 's':
				return ParseString();

			case 'a':
				return ParseArray();
		}

		CLogger::Error("parse error check ParseValue !");

	}

	php_val ParseNumeric()
	{
		php_val val;
		val.value_type = type::type_numeric;

		Expect(':');

		std::string num = ReadUntil(';');
		val.numeric_val = std::stoi(num);

		return val;
	}

	php_val ParseString()
	{
		php_val val;
		val.value_type = type::type_string;

		Expect(':');

		int len = std::stoi(ReadUntil(':'));

		Expect('"');

		val.string_val = input_.substr(pos_, len);

		pos_ += len;

		Expect('"');
		Expect(';');

		return val;
	}

	php_val ParseArray()
	{
		php_val val;
		val.value_type = type::type_array;

		Expect(':');

		int count = std::stoi(ReadUntil(':'));

		Expect('{');

		for (int i = 0; i < count; i++)
		{
			php_val key = ParseValue();
			php_val value = ParseValue();

			if (key.value_type == type::type_numeric)
				val.array_val[key.numeric_val] = value;
		}

		Expect('}');

		return val;
	}
};