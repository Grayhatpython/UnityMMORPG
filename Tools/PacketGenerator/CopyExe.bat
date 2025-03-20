pushd %~dp0

XCOPY /Y ..\Binary\Release\net6.0\PacketGenerator.exe ..\..\Common\Protoc
XCOPY /Y ..\Binary\Release\net6.0\PacketGenerator.dll ..\..\Common\Protoc
XCOPY /Y ..\Binary\Release\net6.0\PacketGenerator.deps.json ..\..\Common\Protoc
XCOPY /Y ..\Binary\Release\net6.0\PacketGenerator.runtimeconfig.json ..\..\Common\Protoc
