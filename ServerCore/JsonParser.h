#pragma once
//	¿œ¥‹..
#include "rapidjson/document.h"

class JsonParser
{
public:
	static bool				ParseFromFile(const CHAR* path, OUT rapidjson::Document& document);

public:
	static int32			GetInt(const rapidjson::Value& obj, const CHAR* member);
	static float			GetFloat(const rapidjson::Value& obj, const CHAR* member);
	static double			GetDouble(const rapidjson::Value& obj, const CHAR* member);
	static std::string		GetString(const rapidjson::Value& obj, const CHAR* member);
	static std::wstring		GetWString(const rapidjson::Value& obj, const CHAR* member);
	static bool				GetBool(const rapidjson::Value& obj, const CHAR* member);
};
