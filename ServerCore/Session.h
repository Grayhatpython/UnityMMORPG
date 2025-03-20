#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"
#include "RecvBuffer.h"
#include "concurrentqueue.h"

/*--------------
	Session
---------------*/

class Service;
class Session : public IocpObject
{
	friend class Listener;
	friend class Service;
	friend class IocpCore;

	enum
	{
		BUFFER_SIZE = 0x10000,
	};

public:
	Session();
	virtual ~Session();

public:
	void Close();

public:
	void					Send(SendBufferRef sendBuffer);
	bool					Connect();
	void					Disconnect(const WCHAR* cause);
	shared_ptr<Service>		GetService() { return _service.lock(); }
	void					SetService(shared_ptr<Service> service) { _service = service; }

public:
	/* ���� ���� */
	void					SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress				GetAddress() { return _netAddress; }
	SOCKET					GetSocket() { return _socket; }
	bool					IsConnected() { return _connected.load(); }
	SessionRef				GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }


private:
	/* �������̽� ���� */
	virtual HANDLE		GetHandle() override;
	virtual void		Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	bool	RegisterConnect();
	bool	RegisterDisconnect();
	void	RegisterPrevRecv();
	void	RegisterRecv();
	void	RegisterSend();

	void	ProcessConnect();
	void	ProcessDisconnect();
	//		TEMP
	void	ProcessPrevRecv(int32 numOfBytes);
	void	ProcessRecv(int32 numOfBytes);
	void	ProcessSend(int32 numOfBytes);

	void	HandleError(int32 errorCode);

protected:
	//	������ �ڵ忡�� �������̵�
	virtual void OnConnected() {}
	virtual int32 OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void OnSend(int32 len) {}
	virtual void OnDisconnected() {}

private:
	weak_ptr<Service>	_service;
	SOCKET				_socket = INVALID_SOCKET;
	NetAddress			_netAddress = {};
	Atomic<bool>		_connected = false;

private:
	USE_LOCK;

	RecvBuffer				_recvBuffer;

	Queue<SendBufferRef>						_sendBufferQueue;
	moodycamel::ConcurrentQueue<SendBufferRef>	_lockFreeSendBufferQueue;
	Atomic<bool>								_isSendRegistered = false;

private:
	//	������ �۽Ű� �޸� ��ø���� �ʰ� ó�� �Ŀ� �ٽ� ���� ����� �Ѵ�.
	RecvEvent		_recvEvent;
	PrevRecvEvent	_prevRecvEvent;
	SendEvent		_sendEvent;
	ConnectEvent	_connectEvent;
	DisonnectEvent	_disconnectEvent;
};

struct PacketHeader
{
	uint16 size = 0;
	uint16 id = 0;
};

class PacketSession : public Session
{
public:
	PacketSession();
	virtual ~PacketSession();

protected:
	virtual int32	OnRecv(BYTE* buffer, int32 len) sealed;
	virtual void	OnRecvPacket(BYTE* buffer, int32 len) abstract;

public:
	PacketSessionRef GetPacketSessionRef() { return static_pointer_cast<PacketSession>(shared_from_this()); }

};