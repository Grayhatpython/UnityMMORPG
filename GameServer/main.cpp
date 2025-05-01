#include "pch.h"
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "GameServer.h"
#include "DBBind.h"
#include "DBConnectionPool.h"
#include "Exception.h"

GameServer* gameServer;

//BOOL CtrlHandler(DWORD CtrlType)
//{
//	switch (CtrlType)
//	{
//		// Handle the CTRL+C signal.
//	case CTRL_C_EVENT:
//	case CTRL_CLOSE_EVENT: // CTRL+CLOSE: confirm! that the user wants to exit.
//	case CTRL_BREAK_EVENT:
//	case CTRL_LOGOFF_EVENT:
//	case CTRL_SHUTDOWN_EVENT:
//	default:
//		gameServer->Close();
//		delete gameServer;
//		gameServer = nullptr;
//		return FALSE; 
//	}
//}

// 다양한 크기의 테스트 클래스
class SmallObject {
	int32 data;
public:
	SmallObject(int32 d = 0) : data(d) { }
	~SmallObject()
	{
		
	}
	int32 getValue() const { return data; }
};

class MediumObject {
	int data[32];  // 약 128바이트
public:
	MediumObject(int d = 0) {
		for (int i = 0; i < 32; ++i) data[i] = d + i;
	}
	int getValue() const {
		int sum = 0;
		for (int i = 0; i < 32; ++i) sum += data[i];
		return sum;
	}
};

class LargeObject {
	int data[256];  // 약 1024바이트
public:
	LargeObject(int d = 0) {
		for (int i = 0; i < 256; ++i) data[i] = d + i;
	}
	int getValue() const {
		int sum = 0;
		for (int i = 0; i < 256; ++i) sum += data[i];
		return sum;
	}
};

// 멀티스레드 테스트
void threadFunction(int id, int iterations) {
	std::cout << "스레드 " << id << " 시작\n";

	for (int i = 0; i < iterations; ++i) {
		// 다양한 객체 할당 테스트
		auto smallPtr = cnew<SmallObject>(i);
		auto mediumPtr = cnew<MediumObject>(i);
		auto largePtr = cnew<LargeObject>(i);

		// 잠시 사용
		int result = smallPtr->getValue() + mediumPtr->getValue() + largePtr->getValue();

		// 해제
		cdelete(smallPtr);
		cdelete(mediumPtr);
		cdelete(largePtr);

		// 스마트 포인터 테스트
		auto sharedSmall = MakeShared<SmallObject>(i);
		auto sharedMedium = MakeShared<MediumObject>(i);
	}

	std::cout << "스레드 " << id << " 종료\n";
}


int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	{
		auto size = sizeof(SmallObject);
		auto small = ObjectPool<SmallObject>::Pop(3);
		ObjectPool<SmallObject>::Push(small);

	}

	{
		auto small = ObjectPool<SmallObject>::MakeShared(3);
	}

	std::cout << "글로벌 메모리 풀 테스트 시작\n";

	const int NUM_THREADS = 4;
	const int ITERATIONS_PER_THREAD = 1000000;

	// 멀티스레딩 테스트
	std::cout << "\n멀티스레딩 테스트 시작 (" << NUM_THREADS << " 스레드, 각 "
		<< ITERATIONS_PER_THREAD << " 반복)...\n";

	std::vector<std::thread> threads;

	auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < NUM_THREADS; ++i) {
		threads.emplace_back(threadFunction, i, ITERATIONS_PER_THREAD);
	}

	for (auto& t : threads) {
		t.join();
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = end - start;

	std::cout << "멀티스레딩 테스트 완료: " << elapsed.count() << "초\n";

	return 0;
}

/*

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(6610);
	std::cout.imbue(std::locale(""));
	std::wcout.imbue(std::locale(""));

	::SetUnhandledExceptionFilter(ExceptionFilter);

	//ASSERT_CRASH(GDBConnectionPool->Connect(1, L"Driver={ODBC Driver 17 for SQL Server};server=(localdb)\\MSSQLLocalDB;database=GameServerDb;trusted_connection=Yes;"));

	//	Exit Function Callback Register
	/*BOOL successd = SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
	ASSERT_CRASH(successd);


	{
		gameServer = new GameServer();

		ASSERT_CRASH(gameServer->Initialize());
		ASSERT_CRASH(gameServer->Start());
		gameServer->DoProcessJob();
	}

	gameServer->Close();
	delete gameServer;

	system("pause");

	return 0;
}



{
	//	DB Test
	ASSERT_CRASH(GDBConnectionPool->Connect(1, L"Driver={ODBC Driver 17 for SQL Server};server=(localdb)\\MSSQLLocalDB;database=GameServerDb;trusted_connection=Yes;"));

	//	Create Table
	{
		auto query = L"									\
			DROP TABLE IF EXISTS [dbo].[Gold];			\
			CREATE TABLE [dbo].[Gold]					\
			(											\
				[id] INT NOT NULL PRIMARY KEY IDENTITY, \
				[gold] INT NULL,						\
				[name] NVARCHAR(50) NULL,				\
				[createDate] DATETIME NULL				\
			);";

		auto dbConntion = GDBConnectionPool->Pop();
		ASSERT_CRASH(dbConntion->Execute(query));
		GDBConnectionPool->Push(dbConntion);
	}

	{
		for (int32 i = 0; i < 3; i++)
		{
			auto dbConntion = GDBConnectionPool->Pop();
			DBBind<3, 0> dbBind(*dbConntion, L"INSERT INTO [dbo].[Gold]([gold], [name], [createDate]) VALUES(?, ?, ?)");

			int32 gold = 100;
			dbBind.BindParam(0, gold);
			WCHAR name[100] = L"홍길동";
			dbBind.BindParam(1, name);
			TIMESTAMP_STRUCT ts = { 2024,5,18 };
			dbBind.BindParam(2, ts);
			ASSERT_CRASH(dbBind.Execute());

			GDBConnectionPool->Push(dbConntion);
		}
	}

	{
		auto dbConntion = GDBConnectionPool->Pop();
		DBBind<1, 4> dbBind(*dbConntion, L"SELECT id, gold, name, createDate FROM [dbo].[gold] WHERE gold = (?)");
		int32 gold = 100;
		dbBind.BindParam(0, gold);

		int32 outId = 0;
		int32 outGold = 0;
		WCHAR outName[100] = { 0, };
		TIMESTAMP_STRUCT outTs = { 0, };
		dbBind.BindCol(0, OUT outId);
		dbBind.BindCol(1, OUT outGold);
		dbBind.BindCol(2, OUT outName);
		dbBind.BindCol(3, OUT outTs);

		ASSERT_CRASH(dbBind.Execute());

		wcout.imbue(std::locale("kor"));

		while (dbBind.Fetch())
		{
			wcout << "id : " << outId << " Gold : " << outGold << " Name : " << outName << endl;
			wcout << "Date : " << outTs.year << "/" << outTs.month << "/" << outTs.day << endl;
		}

		GDBConnectionPool->Push(dbConntion);
	}
}


GRoom->PushReserveJob(1000, []()
	{
		cout << 1000 << endl;
	}
);
GRoom->PushReserveJob(2000, []() {cout << 2000 << endl; });
GRoom->PushReserveJob(3000, []() {cout << 3000 << endl; });
*/