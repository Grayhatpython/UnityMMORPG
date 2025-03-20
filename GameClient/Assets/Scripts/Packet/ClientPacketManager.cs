using Google.Protobuf;
using Google.Protobuf.Protocol;
using ServerCore;
using System;
using System.Collections.Generic;

public enum MsgId
{
	S_CONNECTED = 1,
	C_ENTERGAME = 1,
	S_ENTERGAME = 1,
	S_LEAVEGAME = 1,
	S_SPAWN = 1,
	S_DESPAWN = 1,
	C_MOVE = 1,
	S_MOVE = 1,
}

class PacketManager
{
	#region Singleton
	static PacketManager _instance = new PacketManager();
	public static PacketManager Instance { get { return _instance; } }
	#endregion

	PacketManager()
	{
		Register();
	}

	Dictionary<ushort, Action<PacketSession, ArraySegment<byte>, ushort>> _onRecv = new Dictionary<ushort, Action<PacketSession, ArraySegment<byte>, ushort>>();
	Dictionary<ushort, Action<PacketSession, IMessage>> _handler = new Dictionary<ushort, Action<PacketSession, IMessage>>();
		
	public Action<PacketSession, IMessage, ushort> CustomHandler { get; set; }

	public void Register()
	{		
		_onRecv.Add((ushort)MsgId.S_CONNECTED, MakePacket<S_CONNECTED>);
		_handler.Add((ushort)MsgId.S_CONNECTED, PacketHandler.S_CONNECTEDHandler);		
		_onRecv.Add((ushort)MsgId.S_ENTERGAME, MakePacket<S_ENTERGAME>);
		_handler.Add((ushort)MsgId.S_ENTERGAME, PacketHandler.S_ENTERGAMEHandler);		
		_onRecv.Add((ushort)MsgId.S_LEAVEGAME, MakePacket<S_LEAVEGAME>);
		_handler.Add((ushort)MsgId.S_LEAVEGAME, PacketHandler.S_LEAVEGAMEHandler);		
		_onRecv.Add((ushort)MsgId.S_SPAWN, MakePacket<S_SPAWN>);
		_handler.Add((ushort)MsgId.S_SPAWN, PacketHandler.S_SPAWNHandler);		
		_onRecv.Add((ushort)MsgId.S_DESPAWN, MakePacket<S_DESPAWN>);
		_handler.Add((ushort)MsgId.S_DESPAWN, PacketHandler.S_DESPAWNHandler);		
		_onRecv.Add((ushort)MsgId.S_MOVE, MakePacket<S_MOVE>);
		_handler.Add((ushort)MsgId.S_MOVE, PacketHandler.S_MOVEHandler);
	}

	public void OnRecvPacket(PacketSession session, ArraySegment<byte> buffer)
	{
		ushort count = 0;

		ushort size = BitConverter.ToUInt16(buffer.Array, buffer.Offset);
		count += 2;
		ushort id = BitConverter.ToUInt16(buffer.Array, buffer.Offset + count);
		count += 2;

		Action<PacketSession, ArraySegment<byte>, ushort> action = null;
		if (_onRecv.TryGetValue(id, out action))
			action.Invoke(session, buffer, id);
	}

	void MakePacket<T>(PacketSession session, ArraySegment<byte> buffer, ushort id) where T : IMessage, new()
	{
		T pkt = new T();
		pkt.MergeFrom(buffer.Array, buffer.Offset + 4, buffer.Count - 4);

		if (CustomHandler != null)
		{
			CustomHandler.Invoke(session, pkt, id);
		}
		else
		{
			Action<PacketSession, IMessage> action = null;
			if (_handler.TryGetValue(id, out action))
				action.Invoke(session, pkt);
		}
	}

	public Action<PacketSession, IMessage> GetPacketHandler(ushort id)
	{
		Action<PacketSession, IMessage> action = null;
		if (_handler.TryGetValue(id, out action))
			return action;
		return null;
	}
}