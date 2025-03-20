#include "pch.h"
#include "ServerSession.h"
#include "ServerPacketHandler.h"
#include "ServerSessionManager.h"

ServerSession::~ServerSession()
{
	cout << "~ServerSession" << endl;
}

void  ServerSession::OnConnected()
{
	Protocol::C_LOGIN loginPacket;
	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(loginPacket);
	Send(sendBuffer);

	GServerSessionManager->Add(std::static_pointer_cast<ServerSession>(shared_from_this()));
}

void ServerSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	auto session = GetPacketSessionRef();
	ServerPacketHandler::PacketProcessing(session, buffer, len);
}

void  ServerSession::OnSend(int32 len)
{
	//cout << "OnSend Len = " << len << endl;
}

//	지금은 더미들은 메모리 릭 신경쓰지 않음
void  ServerSession::OnDisconnected()
{
	GServerSessionManager->Remove(std::static_pointer_cast<ServerSession>(shared_from_this()));
}