#include "pch.h"
#include "CoreGlobal.h"
#include "SocketUtils.h"
#include "MemoryPool.h"
#include "ThreadManager.h"
#include "GlobalQueue.h"
#include "JobTimer.h"
#include "DBConnectionPool.h"
#include "Log.h"

ThreadManager*		GThreadManager = nullptr;
Memory*				GMemoryPool = nullptr;
SendBufferManager*	GSendBufferManager = nullptr;
GlobalQueue*		GGlobalQueue = nullptr;
JobTimer*			GJobTimer = nullptr;
DBConnectionPool*	GDBConnectionPool = nullptr;
ThreadHistory*		GThreadHistory = nullptr;

CoreGlobal* GCoreGlobal = new CoreGlobal();


CoreGlobal::CoreGlobal()
{
	SocketUtils::Init();
	GThreadHistory = new ThreadHistory();
	GThreadManager = new ThreadManager();
	GMemoryPool = new Memory();
	GSendBufferManager = new SendBufferManager();
	GGlobalQueue = new GlobalQueue();
	GJobTimer = new JobTimer();
	GDBConnectionPool = new DBConnectionPool();
}

CoreGlobal::~CoreGlobal()
{
}


void CoreGlobal::Clear()
{
	if (GSendBufferManager)
	{
		delete GSendBufferManager;
		GSendBufferManager = nullptr;
	}
	if (GGlobalQueue)
	{
		delete GGlobalQueue;
		GGlobalQueue = nullptr;
	}
	if (GJobTimer)
	{
		delete GJobTimer;
		GJobTimer = nullptr;
	}
	if (GDBConnectionPool)
	{
		delete GDBConnectionPool;
		GDBConnectionPool = nullptr;
	}

	GThreadManager->Close();
	GThreadManager->Join();

	if (GThreadHistory)
	{
		delete GThreadHistory;
		GThreadHistory = nullptr;
	}
	if (GMemoryPool)
	{
		delete GMemoryPool;
		GMemoryPool = nullptr;
	}

	if (GThreadManager)
	{
		delete GThreadManager;
		GThreadManager = nullptr;
	}

	SocketUtils::Clear();
}
