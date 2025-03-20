#pragma once

class SendBufferChunk;
class SendBuffer : public std::enable_shared_from_this<SendBuffer>
{
public:
	SendBuffer(SendBufferChunkRef owner, BYTE* buffer, uint32 reserveSize);
	~SendBuffer();

public:
	BYTE*	Buffer() { return _buffer; }
	uint32	ReserveSize() { return _reserveSize; }
	uint32	WriteSize() { return _writeSize; }
	void	Close(uint32 writeSize);

private:
	BYTE*				_buffer = nullptr;
	//	예약 사이즈
	uint32				_reserveSize = 0;
	//	데이터 쓴 사이즈
	uint32				_writeSize = 0;
	SendBufferChunkRef	_owner;
};

class SendBufferChunk : public std::enable_shared_from_this<SendBufferChunk>
{
	enum
	{
		SEND_BUFFER_CHUNK_SIZE = 0x50000
	};

public:
	SendBufferChunk();
	~SendBufferChunk();

public:
	void			Reset();
	SendBufferRef	Open(uint32 allocSize);
	void			Close(uint32 writeSize);

public:
	bool	IsOpen() { return _isOpen; }
	BYTE*	Buffer() { return &_buffer[_usedSize]; }
	uint32	FreeSize() { return static_cast<uint32>(_buffer.size()) - _usedSize; }

private:
	bool								_isOpen = false;
	uint32								_usedSize = 0;
	Array<BYTE, SEND_BUFFER_CHUNK_SIZE> _buffer = {};
};

class SendBufferManager
{
public:
	SendBufferRef		Open(uint32 size);

private:
	SendBufferChunkRef	Pop();
	void				Push(SendBufferChunkRef buffer);

	static void			PushGlobal(SendBufferChunk* buffer);
	
public:
	void				Close() { _isSendBufferChunkPoolPushed = false; }

private:
	USE_LOCK;
	Vector<SendBufferChunkRef>	_sendBufferChunks;
	static bool					_isSendBufferChunkPoolPushed;
};

/*
SendBufferChunk는 TLS 영역에 있으므로 각 스레드마다 고유하다.
컨텐츠 영역에서 클라이언트 or 서버로 Send 요청 시 SendBufferManager의 
sendBufferChunk를 관리하는 sendBufferChunk Pool에 재사용할게 없는 경우
새로 할당 받거나 만들어 놓은 것을 재사용한다.

SendBufferChunk의 메커니즘은 TLS을 활용하여 락경쟁을 없애고,
소멸 시점에 대해서의 문제를 Ref Count을 활용했다.

컨텐츠 영역에서 Send 요청 시 GSendBufferManager->Open(reserveSize)를 통해서
TLS의 SendBufferChunk에 reserveSize만큼을 할당받는다.
그리고 그 Chunk의 buffer 주소와 writeSize만큼을 표시하는 SendBufferRef를 만들어서
owner를 Chunk로 한다.
그 순간 Chunk의 Ref Count는 2( TLS에서 +1, SendBufferRef의 owner +1)가 된다.
만약에 이 Chunk가 지금 꽉 찾다고 가정하고 다시 Send를 하게 되면 
SendBufferManager에서는 Chunk를 Pool에서 새로 만들거나 있는 것을 받아온다.
그러면 이 순간에 기준에 Send를 해서 써놓은 Chunk의 Ref Count는 -1이 되서
SendBufferRef만 그 Chunk를 가리키고 있다.
이 말인 즉슨 SendBufferRef는 WSASend가 완료되고 Iocp 큐에서 꺠어난 스레드가 
ProcessSend를 통해서 그 해당하는 Session에 대한 sendEvent의 SendBuffers를 클리어하는
시점에 sendBufferRef는 소멸된다.
따라서 아까 그 SendBufferRef를 통해 send를 한 데이터가 완료되기 전까지는 
Chunk는 Ref Count가 유지 되면서 살아있고 , 자동적으로 send가 완료시 
sendBufferRef들이 소멸되면서 Ref Count가 0가 되면 소멸된다.
*/