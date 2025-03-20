#include "pch.h"
#include "ThreadLocal.h"

thread_local uint32					LThreadId = 0;
thread_local uint64					LEndTickCount = 0;
thread_local SendBufferChunkRef		LSendBufferChunk;
thread_local JobQueueBasedObject*	LCurrentExecuteJobQueue = nullptr;

thread_local ThreadCacheMemory*		LThreadCacheMemory = nullptr;
thread_local std::vector<ThreadCallStackInfo>* LThreadCallStackInfo;