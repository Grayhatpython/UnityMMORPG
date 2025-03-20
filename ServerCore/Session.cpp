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

	//	���� RegisterSend�� �ɸ��� ���� ���¶�� �ɾ��ش�.
	//	Send ������ ��Ű�� ����
	//{
	//	WRITE_LOCK;

	//	//	sendBuffer ���� �׾� ���� Queue
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
	//	���� ( ���� ) �ּ�
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

	//	���� ����
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
	//	���� ��� �� ���� ���� Ȯ��
	if (IsConnected() == false)
		return;

	//	Ref Count ���� -> ���� ��� �� ��ü�� ������� ���� ����
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
	//	���� ��� �� ���� ���� Ȯ��
	if (IsConnected() == false)
		return;

	//	Ref Count ���� -> ���� ��� �� ��ü�� ������� ���� ����
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

//	Send ( sendBufferQueue�� Push ) �� ���� �����忡�� ���������� 
//	RegisterSend�� �� �����常 ȣ���ϰ� �ִ�.
void Session::RegisterSend()
{
	if (IsConnected() == false)
		return;

	//	Ref Count ���� -> �۽� ��� �� ��ü�� ������� ���� ����
	_sendEvent.Init();
	_sendEvent.owner = shared_from_this();

	//	���� ������ ( sendBufferQueue )�� -> sendEvent ( sendBuffers )���� Push
	//	WSASend �۽� �ϴ� ���ȿ� sendBuffer�� �������� �ʰ� Ref Count ������Ŵ
	//	Send������ �۽��� �����͸� SendBuffer�� ���� �����ؼ�
	//	Session���� ������ �ִ� sessionBufferQueue�� Push�Ѵ�.
	//	�׸��� RegisterSend�� ����ϴ� �����忡�� sessionBufferQueue �� ����
	//	SendBuffer���� _sendEvent�� sendBuffers�� Push�Ѵ�.
	//{
	//	WRITE_LOCK;

	//	int32 writeSize = 0;
	//	while (_sendBufferQueue.empty() == false)
	//	{
	//		SendBufferRef sendBuffer = _sendBufferQueue.front();

	//		//	�ʹ� ���� �����͸� �۽� �� �� �����Ƿ� 
	//		//	������ �ɰų� �� �� �������
	//		writeSize += sendBuffer->WriteSize();

	//		_sendBufferQueue.pop();
	//		_sendEvent.sendBuffers.push_back(sendBuffer);
	//	}
	//}

	{
		size_t approxSize = _lockFreeSendBufferQueue.size_approx();  // ���� ť ũ�� ��������
		Vector<SendBufferRef> sendBuffers(approxSize);

		size_t count = _lockFreeSendBufferQueue.try_dequeue_bulk(sendBuffers.begin(), sendBuffers.size());  // �뷫���� ������ŭ dequeue

		for (size_t i = 0; i < count; ++i) 
			_sendEvent.sendBuffers.push_back(sendBuffers[i]);
	}

	//	��Ƽ� ������
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
	��� overlapped �۾��� �װ��� ��û�� ������� �۾��� �Ϸ�Ǵ� ���� ����������. 
	completion port������ ��û�� ������ ���� ������ �۾��� �Ϸ�Ǵ� ���� �������� �ʴ´�"
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

//	1. Ŭ���̾�Ʈ�� ������ �پ��� ���
//	2. ������(Ŭ�󸮾�Ʈ) �������� �ٸ� ������ �پ��� ���
void Session::ProcessConnect()
{
	_connectEvent.owner = nullptr;
	_connected.store(true);

	//	Service ���� ���
	GetService()->AddSession(GetSessionRef());

	//	������ �ڵ忡�� �������̵�
	OnConnected();

	RegisterPrevRecv();

	//	���� ���!
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

	//	���� ���
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

	//	������ �ڵ忡�� �������̵�
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);

	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"OnRead Overflow");
		return;
	}

	_recvBuffer.Clean();

	//	���� ���
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

	//	������ �ڵ忡�� �������̵�
	OnSend(numOfBytes);

	//	�� ������ SendBufferQueue�� �ٸ� �����忡�� Push �� �� �����Ƿ� LOCK
	//	SendBufferQueue�� ��������� �ٸ� �����忡�� RegisterSend�� �϶�� �ϰ� ���� ���´�.
	//	sendBuffer�� ���� �ִٸ� ProcessSend�� ȣ���� �����尡 RegisterSend�� �Ѵ�.
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

		//	dataSize�� PacketHeader�� ������ ��ŭ�� �о�� �Ѵ�.
		if (dataSize < sizeof(PacketHeader))
			break;

		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));
		//	dataSize�� ��Ŷ�� ������ ��ŭ�� �о�� �Ѵ�.
		if (dataSize < header.size)
			break;

		//	��Ŷ ����
		OnRecvPacket(&buffer[processLen], header.size);
		processLen += header.size;
	}

	return processLen;
}
