#pragma once
#include "Session.h"

class ClientSession : public PacketSession
{
public:
	~ClientSession();

public:
	virtual void OnConnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;
	virtual void OnDisconnected() override;

public:
	//	������ �ٲ� �� �ִ� ����Ʈ������ 
	//Atomic<PlayerRef>		_player;
	//PlayerRef				_player;


	//Vector<PlayerRef>		_players;	//	Cycle
	//PlayerRef				_currentPlayer;
	//weak_ptr<class Room>	_room;
};
