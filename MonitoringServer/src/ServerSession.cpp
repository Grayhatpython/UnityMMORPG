#include "pch.h"
#include "ServerSession.h"
#include "ServerPacketHandler.h"

ServerSession::~ServerSession()
{
	cout << "~ServerSession" << endl;
}

void  ServerSession::OnConnected()
{

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

}