#include "pch.h"
#include "GlobalQueue.h"

GlobalQueue::GlobalQueue()
{
}

GlobalQueue::~GlobalQueue()
{
}

void GlobalQueue::Push(JobQueueBasedObjectRef jobQueueBasedObject)
{
	_jobQueues.Push(jobQueueBasedObject);
}

JobQueueBasedObjectRef GlobalQueue::Pop()
{
	return _jobQueues.Pop();
}
