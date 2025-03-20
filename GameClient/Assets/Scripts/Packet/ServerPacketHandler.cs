using Google.Protobuf;
using Google.Protobuf.Protocol;
using ServerCore;
using System;
using System.Collections.Generic;

public enum PacketID
{
		PACKET_C_LOGIN = 1,
	PACKET_S_LOGIN = 2,
	PACKET_C_ENTER_GAME = 3,
	PACKET_S_ENTER_GAME = 4,
	PACKET_C_LEAVE_GAME = 5,
	PACKET_S_LEAVE_GAME = 6,
	PACKET_S_SPAWN = 7,
	PACKET_S_DESPAWN = 8,
	PACKET_C_MOVE = 9,
	PACKET_S_MOVE = 10,
	PACKET_C_CHAT = 11,
	PACKET_S_CHAT = 12,

}

class ServerPacketHandler
{
	#region Singleton
	static ServerPacketHandler _instance = new ServerPacketHandler();
	public static ServerPacketHandler Instance { get { return _instance; } }
	#endregion

	ServerPacketHandler()
	{
		Register();
	}

	Dictionary<ushort, Func<PacketSession, ArraySegment<byte>, IMessage>> _makePacket = new Dictionary<ushort, Func<PacketSession, ArraySegment<byte>, IMessage>>();
	Dictionary<ushort, Action<PacketSession, IMessage>> _packetHandler = new Dictionary<ushort, Action<PacketSession, IMessage>>();
		
	public Dictionary<string, ushort> PacketNameToPacketIdTable  { get; private set; } = new Dictionary<string, ushort>();

	public void Register()
	{
				
		_makePacket.Add((ushort)PacketID.PACKET_S_LOGIN, MakePacket<S_LOGIN>);
		_packetHandler.Add((ushort)PacketID.PACKET_S_LOGIN, PacketHandler.S_LOGIN_PacketHandler);
		
		_makePacket.Add((ushort)PacketID.PACKET_S_ENTER_GAME, MakePacket<S_ENTER_GAME>);
		_packetHandler.Add((ushort)PacketID.PACKET_S_ENTER_GAME, PacketHandler.S_ENTER_GAME_PacketHandler);
		
		_makePacket.Add((ushort)PacketID.PACKET_S_LEAVE_GAME, MakePacket<S_LEAVE_GAME>);
		_packetHandler.Add((ushort)PacketID.PACKET_S_LEAVE_GAME, PacketHandler.S_LEAVE_GAME_PacketHandler);
		
		_makePacket.Add((ushort)PacketID.PACKET_S_SPAWN, MakePacket<S_SPAWN>);
		_packetHandler.Add((ushort)PacketID.PACKET_S_SPAWN, PacketHandler.S_SPAWN_PacketHandler);
		
		_makePacket.Add((ushort)PacketID.PACKET_S_DESPAWN, MakePacket<S_DESPAWN>);
		_packetHandler.Add((ushort)PacketID.PACKET_S_DESPAWN, PacketHandler.S_DESPAWN_PacketHandler);
		
		_makePacket.Add((ushort)PacketID.PACKET_S_MOVE, MakePacket<S_MOVE>);
		_packetHandler.Add((ushort)PacketID.PACKET_S_MOVE, PacketHandler.S_MOVE_PacketHandler);
		
		_makePacket.Add((ushort)PacketID.PACKET_S_CHAT, MakePacket<S_CHAT>);
		_packetHandler.Add((ushort)PacketID.PACKET_S_CHAT, PacketHandler.S_CHAT_PacketHandler);


		
        PacketNameToPacketIdTable.Add("C_LOGIN", (ushort)PacketID.PACKET_C_LOGIN);

        PacketNameToPacketIdTable.Add("C_ENTER_GAME", (ushort)PacketID.PACKET_C_ENTER_GAME);

        PacketNameToPacketIdTable.Add("C_LEAVE_GAME", (ushort)PacketID.PACKET_C_LEAVE_GAME);

        PacketNameToPacketIdTable.Add("C_MOVE", (ushort)PacketID.PACKET_C_MOVE);

        PacketNameToPacketIdTable.Add("C_CHAT", (ushort)PacketID.PACKET_C_CHAT);

	}

	public void OnRecvPacket(PacketSession session, ArraySegment<byte> buffer, Action<PacketSession, IMessage, ushort> onRecvFunc = null)
	{
		ushort count = 0;

		ushort size = BitConverter.ToUInt16(buffer.Array, buffer.Offset);
		count += 2;
		ushort id = BitConverter.ToUInt16(buffer.Array, buffer.Offset + count);
		count += 2;

		Func<PacketSession, ArraySegment<byte>, IMessage> func = null;
		if (_makePacket.TryGetValue(id, out func))
		{
			IMessage packet = func.Invoke(session, buffer);
			//	PacketQueue Push
			if (onRecvFunc != null)
				onRecvFunc.Invoke(session, packet, id);
			else
				HandlePacket(session, packet, id);
		}
	}

	T MakePacket<T>(PacketSession session, ArraySegment<byte> buffer) where T : IMessage, new()
	{
		T packet = new T();
		packet.MergeFrom(buffer.Array, buffer.Offset + 4, buffer.Count - 4);
		return packet;
	}

	public void HandlePacket(PacketSession session, IMessage packet, ushort id)
	{
        Action<PacketSession, IMessage> action = null;
        if (_packetHandler.TryGetValue(id, out action))
            action.Invoke(session, packet);
    }
}