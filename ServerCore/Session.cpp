#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

/*--------------
	Session
---------------*/

Session::Session()
	: _recvBuffer(BUFFER_SIZE)
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

void Session::Close()
{
	if(IsConnected())
		RegisterDisconnect();
}

void Session::Send(SendBufferRef sendBuffer)
{
	if (IsConnected() == false)
		return;

	bool isSendRegistered = false;

	//	현재 RegisterSend가 걸리지 않은 상태라면 걸어준다.
	//	Send 순서를 지키기 위해
	//{
	//	WRITE_LOCK;

	//	//	sendBuffer 들을 쌓아 놓는 Queue
	//	_sendBufferQueue.push(sendBuffer);

	//	if (_isSendRegistered.exchange(true) == false)
	//		isSendRegistered = true;
	//}

	{
		_lockFreeSendBufferQueue.enqueue(sendBuffer);

		if (_isSendRegistered.exchange(true) == false)
			isSendRegistered = true;
	}

	if (isSendRegistered)
		RegisterSend();

}

bool Session::Connect()
{
	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	if (_connected.exchange(false) == false)
		return;

	std::wcout << "Disconnect : " << cause << endl;

	RegisterDisconnect();
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	// TODO
	switch (iocpEvent->eventType)
	{
	case EventType::Connect:
		ProcessConnect();
		break;
	case EventType::Disconnect:
		ProcessDisconnect();
		break;
	case EventType::PrevRecv:
		ProcessPrevRecv(numOfBytes);
		break;
	case EventType::Recv:
		ProcessRecv(numOfBytes);
		break;
	case EventType::Send:
		ProcessSend(numOfBytes);
		break;
	default:
		break;
	}
}

bool Session::RegisterConnect()
{
	if (IsConnected())
		return false;

	if (GetService()->GetServiceType() != ServiceType::Client)
		return false;

	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketUtils::BindAnyAddress(_socket, 0/*any port*/) == false)
		return false;

	_connectEvent.Init();
	_connectEvent.owner = shared_from_this();

	DWORD numOfBytes = 0;
	//	상대방 ( 서버 ) 주소
	SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddr();

	if (false == SocketUtils::ConnectEx(_socket, reinterpret_cast<SOCKADDR*>(&sockAddr),
		sizeof(sockAddr), nullptr, 0, &numOfBytes, &_connectEvent))
	{
		auto errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_connectEvent.Init();
			_connectEvent.owner = nullptr;
			HandleError(errorCode);
			return false;
		}
	}

	return true;

}

bool Session::RegisterDisconnect()
{
	_disconnectEvent.Init();
	_disconnectEvent.owner = shared_from_this();

	//	소켓 재사용
	if (false == SocketUtils::DisconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0))
	{
		auto errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_disconnectEvent.Init();
			_disconnectEvent.owner = nullptr;
			HandleError(errorCode);
			return false;
		}
	}

	return true;
}

void Session::RegisterPrevRecv()
{
	//	수신 등록 전 연결 상태 확인
	if (IsConnected() == false)
		return;

	//	Ref Count 증가 -> 수신 등록 후 객체가 사라지는 것을 방지
	_prevRecvEvent.Init();
	_prevRecvEvent.owner = shared_from_this();

	WSABUF wsaBuf;
	wsaBuf.buf = nullptr;
	wsaBuf.len = 0;

	DWORD numOfBytes = 0;
	DWORD flags = 0;
	if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, &numOfBytes, &flags, &_prevRecvEvent, nullptr))
	{
		auto errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_prevRecvEvent.Init();
			_prevRecvEvent.owner = nullptr;
			HandleError(errorCode);
		}
	}
}

void Session::RegisterRecv()
{
	//	수신 등록 전 연결 상태 확인
	if (IsConnected() == false)
		return;

	//	Ref Count 증가 -> 수신 등록 후 객체가 사라지는 것을 방지
	_recvEvent.Init();
	_recvEvent.owner = shared_from_this();

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = static_cast<ULONG>(_recvBuffer.FreeSize());

	DWORD numOfBytes = 0;
	DWORD flags = 0;
	if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, &numOfBytes, &flags, &_recvEvent, nullptr))
	{
		auto errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_recvEvent.Init();
			_recvEvent.owner = nullptr;
			HandleError(errorCode);
		}
	}
}

//	Send ( sendBufferQueue에 Push ) 는 여러 스레드에서 접근하지만 
//	RegisterSend는 한 스레드만 호출하고 있다.
void Session::RegisterSend()
{
	if (IsConnected() == false)
		return;

	//	Ref Count 증가 -> 송신 등록 후 객체가 사라지는 것을 방지
	_sendEvent.Init();
	_sendEvent.owner = shared_from_this();

	//	보낼 데이터 ( sendBufferQueue )를 -> sendEvent ( sendBuffers )으로 Push
	//	WSASend 송신 하는 동안에 sendBuffer가 없어지지 않게 Ref Count 증가시킴
	//	Send에서는 송신할 데이터를 SendBuffer를 만들어서 복사해서
	//	Session마다 가지고 있는 sessionBufferQueue에 Push한다.
	//	그리고 RegisterSend를 담당하는 스레드에서 sessionBufferQueue 에 쌓인
	//	SendBuffer들을 _sendEvent에 sendBuffers로 Push한다.
	//{
	//	WRITE_LOCK;

	//	int32 writeSize = 0;
	//	while (_sendBufferQueue.empty() == false)
	//	{
	//		SendBufferRef sendBuffer = _sendBufferQueue.front();

	//		//	너무 많은 데이터를 송신 할 수 있으므로 
	//		//	제한을 걸거나 할 때 사용하자
	//		writeSize += sendBuffer->WriteSize();

	//		_sendBufferQueue.pop();
	//		_sendEvent.sendBuffers.push_back(sendBuffer);
	//	}
	//}

	{
		size_t approxSize = _lockFreeSendBufferQueue.size_approx();  // 현재 큐 크기 가져오기
		Vector<SendBufferRef> sendBuffers(approxSize);

		size_t count = _lockFreeSendBufferQueue.try_dequeue_bulk(sendBuffers.begin(), sendBuffers.size());  // 대략적인 개수만큼 dequeue

		for (size_t i = 0; i < count; ++i) 
			_sendEvent.sendBuffers.push_back(sendBuffers[i]);
	}

	//	모아서 보내기
	Vector<WSABUF> wsaBufs;
	wsaBufs.reserve(_sendEvent.sendBuffers.size());

	for (auto& sendBuffer : _sendEvent.sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<ULONG>(sendBuffer->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}

	DWORD numOfBytes = 0;

	/*
	모든 overlapped 작업은 그것을 요청한 순서대로 작업이 완료되는 것을 보장하지만. 
	completion port에서는 요청한 순서와 같은 순서로 작업이 완료되는 것을 보장하지 않는다"
	*/
	if (SOCKET_ERROR == ::WSASend(_socket, wsaBufs.data(),
		static_cast<DWORD>(wsaBufs.size()), &numOfBytes, 0, &_sendEvent, nullptr))
	{
		auto errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_sendEvent.Init();
			_sendEvent.owner = nullptr;
			_sendEvent.sendBuffers.clear();
			_isSendRegistered.store(false);
			HandleError(errorCode);
		}
	}
}

//	1. 클라이언트가 서버에 붙었을 경우
//	2. 서버가(클라리언트) 입장으로 다른 서버에 붙었을 경우
void Session::ProcessConnect()
{
	_connectEvent.owner = nullptr;
	_connected.store(true);

	//	Service 세션 등록
	GetService()->AddSession(GetSessionRef());

	//	컨텐츠 코드에서 오버라이딩
	OnConnected();

	RegisterPrevRecv();

	//	수신 등록!
	//RegisterRecv();
}

void Session::ProcessDisconnect()
{
	_disconnectEvent.owner = nullptr;

	SocketUtils::Close(_socket);
	GetService()->ReleaseSession(GetSessionRef());

	OnDisconnected();
}

void Session::ProcessPrevRecv(int32 numOfBytes)
{
	_prevRecvEvent.owner = nullptr;

	//	수신 등록
	RegisterRecv();
}

void Session::ProcessRecv(int32 numOfBytes)
{
	_recvEvent.owner = nullptr;

	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}
	
	if (_recvBuffer.OnWrite(numOfBytes) == false)
	{
		Disconnect(L"OnWrite Overflow");
		return;
	}

	int32 dataSize = _recvBuffer.DataSize();

	//	컨텐츠 코드에서 오버라이딩
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);

	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"OnRead Overflow");
		return;
	}

	_recvBuffer.Clean();

	//	수신 등록
	RegisterPrevRecv();
}

void Session::ProcessSend(int32 numOfBytes)
{
	_sendEvent.owner = nullptr;
	_sendEvent.sendBuffers.clear();

	if (numOfBytes == 0)
	{
		Disconnect(L"Send 0");
		return;
	}

	//	컨텐츠 코드에서 오버라이딩
	OnSend(numOfBytes);

	//	이 찰나에 SendBufferQueue는 다른 스레드에서 Push 할 수 있으므로 LOCK
	//	SendBufferQueue가 비어있으면 다른 스레드에서 RegisterSend를 하라고 하고 빠져 나온다.
	//	sendBuffer가 남아 있다면 ProcessSend를 호출한 스레드가 RegisterSend를 한다.
	WRITE_LOCK;
	if (_sendBufferQueue.empty())
		_isSendRegistered.store(false);
	else
		RegisterSend();
}

void Session::HandleError(int32 errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		Disconnect(L"HandleError");
	default:
		cout << "Handle Error : " << errorCode << endl;
		break;
	}
}

PacketSession::PacketSession()
{
}

PacketSession::~PacketSession()
{
}

int32 PacketSession::OnRecv(BYTE* buffer, int32 len)
{
	int32 processLen = 0;
	while (true)
	{
		int32 dataSize = len - processLen;

		//	dataSize가 PacketHeader의 사이즈 만큼은 읽어야 한다.
		if (dataSize < sizeof(PacketHeader))
			break;

		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));
		//	dataSize가 패킷의 사이즈 만큼은 읽어야 한다.
		if (dataSize < header.size)
			break;

		//	패킷 조립
		OnRecvPacket(&buffer[processLen], header.size);
		processLen += header.size;
	}

	return processLen;
}
