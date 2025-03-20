#pragma once
#include <vector>

extern thread_local uint32										LThreadId;
extern thread_local uint64										LEndTickCount;
extern thread_local SendBufferChunkRef							LSendBufferChunk;
extern thread_local class JobQueueBasedObject*					LCurrentExecuteJobQueue;

extern thread_local class ThreadCacheMemory*					LThreadCacheMemory;
extern thread_local std::vector<struct ThreadCallStackInfo>*	LThreadCallStackInfo;