#pragma once
#include "Protocol.pb.h"

using PacketProcessingFunction = std::function<bool(std::shared_ptr<PacketSession>&, BYTE*, int32)>;
extern PacketProcessingFunction GPacketPacketProcessingFunction[UINT16_MAX];

enum : uint16
{
//	Auto
	PACKET_S_CONNECTED = 1,
	PACKET_C_ENTERGAME = 2,
	PACKET_S_ENTERGAME = 3,
	PACKET_S_LEAVEGAME = 4,
	PACKET_S_SPAWN = 5,
	PACKET_S_DESPAWN = 6,
	PACKET_C_MOVE = 7,
	PACKET_S_MOVE = 8,
	PACKET_S_CPUINFO = 9,

};

//	Custom Packet Processing Function
bool Packet_Processing_Function_Undefined(std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len);
//	Auto
bool S_CONNECTED_Packet_Processing_Function(std::shared_ptr<PacketSession>& session, Protocol::S_CONNECTED& packet);
bool S_ENTERGAME_Packet_Processing_Function(std::shared_ptr<PacketSession>& session, Protocol::S_ENTERGAME& packet);
bool S_LEAVEGAME_Packet_Processing_Function(std::shared_ptr<PacketSession>& session, Protocol::S_LEAVEGAME& packet);
bool S_SPAWN_Packet_Processing_Function(std::shared_ptr<PacketSession>& session, Protocol::S_SPAWN& packet);
bool S_DESPAWN_Packet_Processing_Function(std::shared_ptr<PacketSession>& session, Protocol::S_DESPAWN& packet);
bool S_MOVE_Packet_Processing_Function(std::shared_ptr<PacketSession>& session, Protocol::S_MOVE& packet);
bool S_CPUINFO_Packet_Processing_Function(std::shared_ptr<PacketSession>& session, Protocol::S_CPUINFO& packet);


class ServerPacketHandler
{
public:
	static void Initialize()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketPacketProcessingFunction[i] = Packet_Processing_Function_Undefined;

		//	Register Function
		//	Auto
		GPacketPacketProcessingFunction[PACKET_S_CONNECTED] = [](std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len) { return TPacketProcessing<Protocol::S_CONNECTED>(S_CONNECTED_Packet_Processing_Function, session, buffer, len); };
		GPacketPacketProcessingFunction[PACKET_S_ENTERGAME] = [](std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len) { return TPacketProcessing<Protocol::S_ENTERGAME>(S_ENTERGAME_Packet_Processing_Function, session, buffer, len); };
		GPacketPacketProcessingFunction[PACKET_S_LEAVEGAME] = [](std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len) { return TPacketProcessing<Protocol::S_LEAVEGAME>(S_LEAVEGAME_Packet_Processing_Function, session, buffer, len); };
		GPacketPacketProcessingFunction[PACKET_S_SPAWN] = [](std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len) { return TPacketProcessing<Protocol::S_SPAWN>(S_SPAWN_Packet_Processing_Function, session, buffer, len); };
		GPacketPacketProcessingFunction[PACKET_S_DESPAWN] = [](std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len) { return TPacketProcessing<Protocol::S_DESPAWN>(S_DESPAWN_Packet_Processing_Function, session, buffer, len); };
		GPacketPacketProcessingFunction[PACKET_S_MOVE] = [](std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len) { return TPacketProcessing<Protocol::S_MOVE>(S_MOVE_Packet_Processing_Function, session, buffer, len); };
		GPacketPacketProcessingFunction[PACKET_S_CPUINFO] = [](std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len) { return TPacketProcessing<Protocol::S_CPUINFO>(S_CPUINFO_Packet_Processing_Function, session, buffer, len); };

	}

	static bool PacketProcessing(std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len)
	{
		PacketHeader* packetHeader = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketPacketProcessingFunction[packetHeader->id](session, buffer, len);
	}

public:
	//	Auto
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::C_ENTERGAME& packet) { return TMakeSendBuffer(packet, PACKET_C_ENTERGAME); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::C_MOVE& packet) { return TMakeSendBuffer(packet, PACKET_C_MOVE); }


private:
	template<typename PacketType, typename ProcessFunction>
	static bool TPacketProcessing(ProcessFunction function, std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len)
	{
		PacketType packet;

		PacketHeader* packetHeader = reinterpret_cast<PacketHeader*>(buffer);
		const uint16 dataSize = packetHeader->size - sizeof(PacketHeader);

		if (packet.ParseFromArray(&packetHeader[1], dataSize) == false)
			return false;

		return function(session, packet);
	}

	template<typename T>
	static std::shared_ptr<SendBuffer> TMakeSendBuffer(T& packet, uint16 packetId)
	{
		const uint16 dataSize = static_cast<uint16>(packet.ByteSizeLong());
		const uint16 packetTotalSize = dataSize + sizeof(PacketHeader);

		auto sendBuffer = GSendBufferManager->Open(packetTotalSize);
		PacketHeader* packetHeader = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		packetHeader->id = packetId;
		packetHeader->size = packetTotalSize;

		ASSERT_CRASH(packet.SerializeToArray(&packetHeader[1], dataSize));

		sendBuffer->Close(packetTotalSize);
		return sendBuffer;
	}
};