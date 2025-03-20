#pragma once
#include "Session.h"

class ServerSession : public PacketSession
{
public:
	~ServerSession();

public:
	virtual void OnConnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;
	virtual void OnDisconnected() override;

public:
	PlayerRef	_player;
};
