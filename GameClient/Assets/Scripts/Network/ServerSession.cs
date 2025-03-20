using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using ServerCore;
using Google.Protobuf.Protocol;
using Google.Protobuf;

namespace DummyClient
{
	class ServerSession : PacketSession
	{
		public override void OnConnected(EndPoint endPoint)
		{
			Console.WriteLine($"OnConnected : {endPoint}");
			C_LOGIN loginPacket= new C_LOGIN();
			Managers.Network.Send(loginPacket);
        }

        public override void OnDisconnected(EndPoint endPoint)
		{
			Console.WriteLine($"OnDisconnected : {endPoint}");
		}

		public override void OnRecvPacket(ArraySegment<byte> buffer)
		{
			ServerPacketHandler.Instance.OnRecvPacket(this, buffer, (session, packet, id) => PacketQueue.Instance.Push(id, packet));
		}

		public override void OnSend(int numOfBytes)
		{
			//Console.WriteLine($"Transferred bytes: {numOfBytes}");
		}
	}
}
