#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "ClientPacketHandler.h"
#include "GameServer.h"
#include "ClientSession.h"
#include "Monitoring.h"
#include "Manager.h"
#include <DBConnectionPool.h>

GameServer::~GameServer()
{

}

bool GameServer::Initialize()
{
	Manager::GetInstance()->Initialize();
	Manager::GetInstance()->GetConfigManager()->LoadServerConfigInfo("../Common/config.json");

	ClientPacketHandler::Initialize();

	const auto& serverConfigInfo = Manager::GetInstance()->GetConfigManager()->GetServerConfigInfo();

	ASSERT_CRASH(GDBConnectionPool->Connect(1, serverConfigInfo.connectionString.c_str()));

	_service = MakeShared<ServerService>(
		NetAddress(serverConfigInfo.ip, serverConfigInfo.port),
		MakeShared<IocpCore>(),
		MakeShared<ClientSession>, // TODO : SessionManager 등
		10);

	ASSERT_CRASH(_service);

	return true;
}

bool GameServer::Start()
{
	ASSERT_CRASH(_service->Start());

	return true;
}

void GameServer::DoProcessJob()
{
	/*Monitoring m;
	{
		GThreadManager->Launch([&m]()
			{
				m.Print();
				std::this_thread::sleep_for(1s);
			}, L"Monitoring Thread");
	}*/

	for (int32 i = 0; i < 5; i++)
	{
		//	Ref Count No Add
		GThreadManager->Launch([service = _service]()
			{
				LEndTickCount = ::GetTickCount64() + PROCESS_TICK;

				//	I/O ( JobQueue Push MultiThread )
				//	Game Logic ( JobQueue Execute -> One Thread )
				if(service)
					service->GetIocpCore()->Dispatch(10);

				//	Global Reserve Jobs Push JobQueue
				//ThreadManager::DistributeReserveJobs();

				//	Game Logic
				//ThreadManager::DoGlobalJobQueueExecute();

			},L"Worker Thread");
	}

	auto dbManager = Manager::GetDBManager();
	GThreadManager->Launch([db = dbManager]()
		{
			db->PopAllExecute();
			std::this_thread::sleep_for(100ms);
		}, L"DB Thread");

	//Manager::GetDBManager()->Test();

	//	Thread Join
	//GThreadManager->Join();
	while (_stopped == false)
	{
		//GRoom->Update();
		
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
			break;

		std::this_thread::sleep_for(10ms);
	}
}

void GameServer::Close()
{
	_stopped = true;
 
	_service->CloseService();
	GSendBufferManager->Close();
	_service = nullptr;

	Manager::GetInstance()->Clear();

	GCoreGlobal->Clear();
	delete GCoreGlobal;
	GCoreGlobal = nullptr;

	google::protobuf::ShutdownProtobufLibrary();
}
