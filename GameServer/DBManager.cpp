#include "pch.h"
#include "DBManager.h"
#include <DBBind.h>
#include <DBConnectionPool.h>
#include "Random.h"

void DBManager::Test()
{
	auto dbConntion = GDBConnectionPool->Pop();
	dbConntion->Unbind();
	DBBind<1, 0> dbBind(*dbConntion, L"EXEC usp_CreatePlayer ?");

	int64 accountDbId = static_cast<int64>(Random::GetRandom(0, 100));
	dbBind.BindParam(0, accountDbId);
	ASSERT_CRASH(dbBind.Execute());

	GDBConnectionPool->Push(dbConntion);
}
