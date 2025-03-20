#include "pch.h"
#include "ConfigManager.h"
#include <JsonParser.h>
#include <UtilityHelper.h>

bool ConfigManager::LoadServerConfigInfo(const std::string& filepath)
{
	rapidjson::Document document;
	JsonParser::ParseFromFile(filepath.c_str(), document);

	if (document.HasMember("ip") && document["ip"].IsString())
	{
		std::string ip = document["ip"].GetString();
		_serverConfigInfo.ip = UtilityHelper::ConvertUTF8ToUnicode(ip);
	}
	if (document.HasMember("port") && document["port"].IsString())
		_serverConfigInfo.port = std::stoi(document["port"].GetString());
	if (document.HasMember("dataPath") && document["dataPath"].IsString())
	{
		std::string dataPath = document["dataPath"].GetString();
		_serverConfigInfo.dataPath = UtilityHelper::ConvertUTF8ToUnicode(dataPath);
	}
	if (document.HasMember("connectionString") && document["connectionString"].IsString())
	{
		std::string connectionString = document["connectionString"].GetString();
		_serverConfigInfo.connectionString = UtilityHelper::ConvertUTF8ToUnicode(connectionString);
	}

	return true;
}