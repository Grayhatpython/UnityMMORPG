#pragma once

extern class ThreadManager*		GThreadManager;
extern class GlobalMemoryPool*		GMemoryPool;
extern class SendBufferManager* GSendBufferManager;
extern class GlobalQueue*		GGlobalQueue;
extern class JobTimer*			GJobTimer;
extern class DBConnectionPool*	GDBConnectionPool;
extern class ThreadHistory*		GThreadHistory;

class CoreGlobal
{
public:
	CoreGlobal();
	~CoreGlobal();

public:
	void Clear();
};

extern CoreGlobal* GCoreGlobal;