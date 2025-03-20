#include "pch.h"
#include "Manager.h"
#include "Random.h"

std::unique_ptr<Manager>	Manager::S_Instance = nullptr;
std::once_flag				Manager::S_InitFlag;

void Manager::Initialize()
{
    Random::Initialize();

    _db = std::make_shared<DBManager>();

    _data = std::make_unique<DataManager>();
    _config = std::make_unique<ConfigManager>();
    _clientSession = std::make_unique<ClientSessionManager>();
}

void Manager::Clear()
{
    if (S_Instance)
    {
        S_Instance->_db.reset();                //  DBManager ����
        S_Instance->_data.reset();              //  DataManager ����
        S_Instance->_config.reset();            //  ConfigManager ����
        S_Instance->_clientSession.reset();     //  ClientSessionManager ����
        S_Instance.reset();                     //  Manager ����  
    }
}
