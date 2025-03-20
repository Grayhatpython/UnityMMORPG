#include "pch.h"
#include "JsonParser.h"
#include "UtilityHelper.h"
#include <filesystem>
#include <fstream>

bool JsonParser::ParseFromFile(const CHAR* path, OUT rapidjson::Document& document)
{
	std::vector<CHAR> data;
	std::filesystem::path filePath{ path };

	const uint32 fileSize = static_cast<uint32>(std::filesystem::file_size(filePath));
	data.resize(fileSize);

	std::basic_ifstream<CHAR> inputfStream{ filePath };
	inputfStream.read(&data[0], fileSize);

	// Load raw data into RapidJSON document
	document.Parse(data.data());
	assert(document.HasParseError() == false);

	return true;
}

//	수정 필요
int32 JsonParser::GetInt(const rapidjson::Value& obj, const CHAR* member)
{
	auto iter = obj.FindMember(member);
	assert(iter != obj.MemberEnd());
	
	auto& value = iter->value;
	assert(value.IsInt());

	return value.GetInt();
}

float JsonParser::GetFloat(const rapidjson::Value& obj, const CHAR* member)
{
	auto iter = obj.FindMember(member);
	assert(iter != obj.MemberEnd());

	auto& value = iter->value;
	assert(value.IsFloat());

	return value.GetFloat();
}

double JsonParser::GetDouble(const rapidjson::Value& obj, const CHAR* member)
{
	auto iter = obj.FindMember(member);
	assert(iter != obj.MemberEnd());

	auto& value = iter->value;
	assert(value.IsDouble());

	return value.GetDouble();
}

std::string	JsonParser::GetString(const rapidjson::Value& obj, const CHAR* member)
{
	auto iter = obj.FindMember(member);
	assert(iter != obj.MemberEnd());

	auto& value = iter->value;
	assert(value.IsString());

	auto string = value.GetString();

	return string;
}

std::wstring JsonParser::GetWString(const rapidjson::Value& obj, const CHAR* member)
{
	auto iter = obj.FindMember(member);
	assert(iter != obj.MemberEnd());

	auto& value = iter->value;
	assert(value.IsString());

	auto string = value.GetString();

	return UtilityHelper::ConvertUTF8ToUnicode(value.GetString());
}

bool JsonParser::GetBool(const rapidjson::Value& obj, const CHAR* member)
{
	auto iter = obj.FindMember(member);
	assert(iter != obj.MemberEnd());

	auto& value = iter->value;
	assert(value.IsBool());

	return value.GetBool();
}
