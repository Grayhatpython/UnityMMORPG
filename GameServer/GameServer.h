#pragma once

class GameServer
{
	enum
	{
		PROCESS_TICK = 64,
	};

public:
	~GameServer();

public:
	bool Initialize();
	bool Start();
	void DoProcessJob();
	void Close();

private:
	ServerServiceRef	_service;
	bool				_stopped = false;
};