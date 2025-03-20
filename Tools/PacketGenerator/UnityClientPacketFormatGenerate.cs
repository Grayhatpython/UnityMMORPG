using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PacketGenerator
{
    class UnityClientPacketFormatGenerate
    {
       string clientPacketManager = string.Empty;
       string gameServerPacketManager = string.Empty;

       string clientMsgIdList = string.Empty;
       string gameServerMsgIdList = string.Empty;

        public void Generate()
        {
#if DEBUG
            string protoPath = "../../../../Common/Protoc/Protocol.proto";
#else
            string protoPath = "./Protocol.proto";
#endif
            foreach (string line in File.ReadAllLines(protoPath))
            {
                string[] names = line.Split(" ");
                if (names.Length == 0)
                    continue;

                if (!names[0].StartsWith("message"))
                    continue;

                ParsePacket(names[1]);
            }

            string clientManagerText = string.Format(UnityClientPacketFormat.managerFormat, clientMsgIdList, clientPacketManager);
            File.WriteAllText("ServerPacketManager.cs", clientManagerText);
        }

        public void ParsePacket(string name)
        {
            int packetStartNumber = 1;

            if (name.StartsWith("S_")) // GameServer -> Client
            {
                clientPacketManager += string.Format(UnityClientPacketFormat.managerRegisterFormat, name);
                gameServerMsgIdList += string.Format(UnityClientPacketFormat.msgIdRegisterFormat, name, packetStartNumber);
                clientMsgIdList += string.Format(UnityClientPacketFormat.msgIdRegisterFormat, name, packetStartNumber);
                packetStartNumber++;
            }
            else if (name.StartsWith("C_")) // Client -> GameServer
            {
                gameServerPacketManager += string.Format(UnityClientPacketFormat.managerRegisterFormat, name);
                clientMsgIdList += string.Format(UnityClientPacketFormat.msgIdRegisterFormat, name, packetStartNumber);
                gameServerMsgIdList += string.Format(UnityClientPacketFormat.msgIdRegisterFormat, name, packetStartNumber);
                packetStartNumber++;
            }
        }

    }
}
