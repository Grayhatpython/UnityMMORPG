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
bool C_ENTERGAME_Packet_Processing_Function(std::shared_ptr<PacketSession>& session, Protocol::C_ENTERGAME& packet);
bool C_MOVE_Packet_Processing_Function(std::shared_ptr<PacketSession>& session, Protocol::C_MOVE& packet);


class ClientPacketHandler
{
public:
	static void Initialize()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketPacketProcessingFunction[i] = Packet_Processing_Function_Undefined;

		//	Register Function
		//	Auto
		GPacketPacketProcessingFunction[PACKET_C_ENTERGAME] = [](std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len) { return TPacketProcessing<Protocol::C_ENTERGAME>(C_ENTERGAME_Packet_Processing_Function, session, buffer, len); };
		GPacketPacketProcessingFunction[PACKET_C_MOVE] = [](std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len) { return TPacketProcessing<Protocol::C_MOVE>(C_MOVE_Packet_Processing_Function, session, buffer, len); };

	}

	static bool PacketProcessing(std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len)
	{
		PacketHeader* packetHeader = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketPacketProcessingFunction[packetHeader->id](session, buffer, len);
	}

public:
	//	Auto
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::S_CONNECTED& packet) { return TMakeSendBuffer(packet, PACKET_S_CONNECTED); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::S_ENTERGAME& packet) { return TMakeSendBuffer(packet, PACKET_S_ENTERGAME); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::S_LEAVEGAME& packet) { return TMakeSendBuffer(packet, PACKET_S_LEAVEGAME); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::S_SPAWN& packet) { return TMakeSendBuffer(packet, PACKET_S_SPAWN); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::S_DESPAWN& packet) { return TMakeSendBuffer(packet, PACKET_S_DESPAWN); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::S_MOVE& packet) { return TMakeSendBuffer(packet, PACKET_S_MOVE); }
	static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::S_CPUINFO& packet) { return TMakeSendBuffer(packet, PACKET_S_CPUINFO); }


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