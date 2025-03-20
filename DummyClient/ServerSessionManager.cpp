#include "pch.h"
#include "ServerSessionManager.h"
#include "ServerSession.h"
#include "Player.h"
#include "ServerPacketHandler.h"
#include "Protocol.pb.h"

ServerSessionManager* GServerSessionManager = nullptr;

void ServerSessionManager::Add(ServerSessionRef session)
{
	WRITE_LOCK;
	_sessions.insert(session);
}

void ServerSessionManager::Remove(ServerSessionRef session)
{
	WRITE_LOCK;
	_sessions.erase(session);
}

void ServerSessionManager::Broadcast(SendBufferRef sendBuffer)
{
	WRITE_LOCK;
	for (auto& session : _sessions)
	{
		session->Send(sendBuffer);
	}
}

//	Move Test
void ServerSessionManager::BroadcastMove()
{
	WRITE_LOCK;
	for (auto& session : _sessions)
	{
		auto randomYaw = Utils::GetRandom(0.f, 360.f);
		session->_player->_positionInfo->set_yaw(randomYaw);
		Protocol::C_MOVE movePacket;
		auto positionInfo = movePacket.mutable_positioninfo();
		positionInfo->set_objectid(session->_player->_id);
		positionInfo->set_yaw(randomYaw);
		positionInfo->set_state(Protocol::MOVE_STATE_RUN);

		auto sendBuffer = ServerPacketHandler::MakeSendBuffer(movePacket);
		session->Send(sendBuffer);
	}
}

//	Idle Test
void ServerSessionManager::BroadcastIdle()
{
	WRITE_LOCK;
	for (auto& session : _sessions)
	{
		auto yaw = session->_player->_positionInfo->yaw();
		Protocol::C_MOVE movePacket;
		auto positionInfo = movePacket.mutable_positioninfo();
		positionInfo->set_objectid(session->_player->_id);
		positionInfo->set_yaw(yaw);
		positionInfo->set_state(Protocol::MOVE_STATE_IDLE);

		auto sendBuffer = ServerPacketHandler::MakeSendBuffer(movePacket);
		session->Send(sendBuffer);
	}
}
