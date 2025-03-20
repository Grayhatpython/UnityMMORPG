#pragma once

#include "DBManager.h"
#include "DataManager.h"
#include "ConfigManager.h"
#include "ClientSessionManager.h"

class DBManager;
class Manager
{
private:
	Manager() {};

public:
	//	TODO
	static Manager* GetInstance()
	{
		std::call_once(S_InitFlag, []() {
			S_Instance = std::unique_ptr<Manager>(new Manager());
			});
		return S_Instance.get();
	}

	static std::shared_ptr<DBManager>		GetDBManager() { return GetInstance()->_db; }

	static DataManager*						GetDataManager() { return GetInstance()->_data.get(); }
	static ConfigManager*					GetConfigManager() { return GetInstance()->_config.get(); }
	static ClientSessionManager*			GetClientSessionManager() { return GetInstance()->_clientSession.get(); }

public:
	void Initialize();
	void Clear();

public:
	Manager(const Manager&) = delete;
	Manager& operator=(const Manager&) = delete;

private:
	static std::unique_ptr<Manager> S_Instance;
	static std::once_flag			S_InitFlag;

	std::shared_ptr<DBManager>				_db;

	std::unique_ptr<DataManager>			_data;
	std::unique_ptr<ConfigManager>			_config;
	std::unique_ptr<ClientSessionManager>	_clientSession;
};

