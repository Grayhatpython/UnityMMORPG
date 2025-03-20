#pragma once

class ServerSession;

using ServerSessionRef = shared_ptr<ServerSession>;

class ServerSessionManager
{
public:
	void Add(ServerSessionRef session);
	void Remove(ServerSessionRef session);
	void Broadcast(SendBufferRef sendBuffer);

	//	TEMP
	void BroadcastMove();
	void BroadcastIdle();

private:
	USE_LOCK;
	Set<ServerSessionRef> _sessions;
};

extern ServerSessionManager* GServerSessionManager;