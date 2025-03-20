
namespace PacketGenerator
{
    internal class Program
    {

        static void Main(string[] args)
        {

            ServerDummyPacketFormatGenerate serverDummyacketFormatGenerate = new ServerDummyPacketFormatGenerate();
            serverDummyacketFormatGenerate.Generate();

            UnityClientPacketFormatGenerate unityClientPacketFormatGenerate = new UnityClientPacketFormatGenerate();
            unityClientPacketFormatGenerate.Generate();
        }
    }
}