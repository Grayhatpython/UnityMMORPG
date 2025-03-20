#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#include <CorePch.h>
#include <iostream>
#include "ThreadManager.h"
#include "Service.h"
#include "ServerSession.h"
#include "ServerPacketHandler.h"
#include "MonitoringServer.h"

int main()
{
	std::cout.imbue(std::locale(""));
	std::wcout.imbue(std::locale(""));

	ServerPacketHandler::Initialize();
	GMonitoringServer.Initialize();

	ClientServiceRef service = MakeShared<ClientService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<ServerSession >, // TODO : SessionManager 등
		1);

	ASSERT_CRASH(service->Start());

	GThreadManager->Launch([=]()
		{
			service->GetIocpCore()->Dispatch();
		});

	GMonitoringServer.Update();

	GThreadManager->Join();
}