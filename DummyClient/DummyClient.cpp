#include "pch.h"
#include <iostream>
#include "ThreadManager.h"
#include "Service.h"
#include "ServerSession.h"
#include "ServerPacketHandler.h"
#include "ServerSessionManager.h"

int main()
{
	ServerPacketHandler::Initialize();

	GServerSessionManager = new ServerSessionManager();


	ClientServiceRef service = MakeShared<ClientService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<ServerSession >, // TODO : SessionManager 등
		500);

	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([=]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}
			});
	}

	this_thread::sleep_for(6s);

	while (true)
	{
		GServerSessionManager->BroadcastMove();
		std::this_thread::sleep_for(500ms);
		GServerSessionManager->BroadcastIdle();
		std::this_thread::sleep_for(5000ms);
	}

	GThreadManager->Join();

	delete GServerSessionManager;
	GServerSessionManager = nullptr;
}