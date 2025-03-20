using ServerCore;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using UnityEngine;
using DummyClient;
using System;
using UnityEngine.UIElements;
using Google.Protobuf;
using Google.Protobuf.Protocol;
using Unity.VisualScripting;

public class NetworkManager
{
    ServerSession _session = new ServerSession();

    public void Send<T>(T packet) where T : IMessage, new()
    {
        Type type = typeof(T);
        string typeName = type.Name;
        ushort packetId = ServerPacketHandler.Instance.PacketNameToPacketIdTable[typeName];

        ushort size = (ushort)packet.CalculateSize();
        byte[] sendBuffer = new byte[size + 4];
        Array.Copy(BitConverter.GetBytes(size + 4), 0, sendBuffer, 0, sizeof(ushort));
        Array.Copy(BitConverter.GetBytes(packetId), 0, sendBuffer, 2, sizeof(ushort));
        Array.Copy(packet.ToByteArray(), 0, sendBuffer, 4, size);

        Send(new ArraySegment<byte>(sendBuffer));
    }

    public void Send(ArraySegment<byte> sendBuffer)
    {
        _session.Send(sendBuffer);
    }

    public void Start()
    {
        // DNS (Domain Name System)
        string host = Dns.GetHostName();
        IPHostEntry ipHost = Dns.GetHostEntry(host);
        //  TEMP
        //IPAddress[] ipv4Address = Array.FindAll(Dns.GetHostEntry(string.Empty).AddressList, a => a.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork);
       
        IPEndPoint endPoint = new IPEndPoint(IPAddress.Loopback, 7777);

        Connector connector = new Connector();

        connector.Connect(endPoint,
            () => { return _session; },
            1);

    }

    public void Update()
    {
        List<PacketMessage> packetList = PacketQueue.Instance.PopAll();
        foreach(PacketMessage packet in packetList)
            ServerPacketHandler.Instance.HandlePacket(_session, packet.Message, packet.Id);

        //TEMP
        if (Input.GetKey(KeyCode.F1))
        {
            C_LEAVE_GAME leaveGamePacket = new C_LEAVE_GAME();
            Send(leaveGamePacket);
        }
    }
}
