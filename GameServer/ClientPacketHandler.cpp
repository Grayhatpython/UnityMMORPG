#include "pch.h"
#include "ClientPacketHandler.h"
#include "ClientSession.h"

PacketProcessingFunction GPacketPacketProcessingFunction[UINT16_MAX];

bool Packet_Processing_Function_Undefined(std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len)
{
	return true;
}

//bool C_TEST_Packet_Processing_Function(std::shared_ptr<PacketSession>& session, Protocol::C_TEST& packet)
//{
//	//	Client -> Login Server -> Game Server
//	//	인증을 확인하기 위해 Redis? 
//	//	DB에서 Account 정보
//
//	std::cout << "헤헤 : " << packet.temp() << std::endl;
//
//	return true;
//}

bool C_ENTERGAME_Packet_Processing_Function(std::shared_ptr<PacketSession>& session, Protocol::C_ENTERGAME& packet)
{

	return true;
}

bool C_MOVE_Packet_Processing_Function(std::shared_ptr<PacketSession>& session, Protocol::C_MOVE& packet)
{
	return true;
}
