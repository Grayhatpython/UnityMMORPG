using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PacketGenerator
{
    internal class ServerDummyPacketFormat
    {
        public static string fileFormat =
@"#pragma once
#include ""Protocol.pb.h""

using PacketProcessingFunction = std::function<bool(std::shared_ptr<PacketSession>&, BYTE*, int32)>;
extern PacketProcessingFunction GPacketPacketProcessingFunction[UINT16_MAX];

enum : uint16
{{
//	Auto
{0}
}};

//	Custom Packet Processing Function
bool Packet_Processing_Function_Undefined(std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len);
//	Auto
{1}

class {2}
{{
public:
	static void Initialize()
	{{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketPacketProcessingFunction[i] = Packet_Processing_Function_Undefined;

		//	Register Function
		//	Auto
{3}
	}}

	static bool PacketProcessing(std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len)
	{{
		PacketHeader* packetHeader = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketPacketProcessingFunction[packetHeader->id](session, buffer, len);
	}}

public:
	//	Auto
{4}

private:
	template<typename PacketType, typename ProcessFunction>
	static bool TPacketProcessing(ProcessFunction function, std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len)
	{{
		PacketType packet;

		PacketHeader* packetHeader = reinterpret_cast<PacketHeader*>(buffer);
		const uint16 dataSize = packetHeader->size - sizeof(PacketHeader);

		if (packet.ParseFromArray(&packetHeader[1], dataSize) == false)
			return false;

		return function(session, packet);
	}}

	template<typename T>
	static std::shared_ptr<SendBuffer> TMakeSendBuffer(T& packet, uint16 packetId)
	{{
		const uint16 dataSize = static_cast<uint16>(packet.ByteSizeLong());
		const uint16 packetTotalSize = dataSize + sizeof(PacketHeader);

		auto sendBuffer = GSendBufferManager->Open(packetTotalSize);
		PacketHeader* packetHeader = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		packetHeader->id = packetId;
		packetHeader->size = packetTotalSize;

		ASSERT_CRASH(packet.SerializeToArray(&packetHeader[1], dataSize));

		sendBuffer->Close(packetTotalSize);
		return sendBuffer;
	}}
}};";

        public static string packetEnumFormat =
@"PACKET_{0} = {1},";

        public static string packetProcessingFunctionFormat =
@"bool {0}_Packet_Processing_Function(std::shared_ptr<PacketSession>& session, Protocol::{1}& packet);";

        public static string packetProcessingFunctionRegisterFormat =
@"GPacketPacketProcessingFunction[PACKET_{0}] = [](std::shared_ptr<PacketSession>& session, BYTE* buffer, int32 len) {{ return TPacketProcessing<Protocol::{1}>({2}_Packet_Processing_Function, session, buffer, len); }};";

        public static string makePacketToSendBufferFormat =
@"static std::shared_ptr<SendBuffer> MakeSendBuffer(Protocol::{0}& packet) {{ return TMakeSendBuffer(packet, PACKET_{1}); }}";
    }
}
