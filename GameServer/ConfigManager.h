#pragma once

struct ServerConfigInfo
{
	std::wstring ip;
	int32 port = 0;
	std::wstring dataPath;
	std::wstring connectionString;
};

class ConfigManager
{
public:
	bool					LoadServerConfigInfo(const std::string& filepath);
	
public:
	const ServerConfigInfo& GetServerConfigInfo() const { return _serverConfigInfo; }

private:
	ServerConfigInfo		_serverConfigInfo;
};

