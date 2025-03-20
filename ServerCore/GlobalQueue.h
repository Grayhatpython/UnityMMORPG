#pragma once

class GlobalQueue
{
public:
	GlobalQueue();
	~GlobalQueue();

public:
	void					Push(JobQueueBasedObjectRef jobQueueBasedObject);
	JobQueueBasedObjectRef	Pop();

private:
	LockQueue<JobQueueBasedObjectRef> _jobQueues;
};

