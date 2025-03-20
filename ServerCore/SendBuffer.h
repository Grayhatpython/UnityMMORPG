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
	//	���� ������
	uint32				_reserveSize = 0;
	//	������ �� ������
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
SendBufferChunk�� TLS ������ �����Ƿ� �� �����帶�� �����ϴ�.
������ �������� Ŭ���̾�Ʈ or ������ Send ��û �� SendBufferManager�� 
sendBufferChunk�� �����ϴ� sendBufferChunk Pool�� �����Ұ� ���� ���
���� �Ҵ� �ްų� ����� ���� ���� �����Ѵ�.

SendBufferChunk�� ��Ŀ������ TLS�� Ȱ���Ͽ� �������� ���ְ�,
�Ҹ� ������ ���ؼ��� ������ Ref Count�� Ȱ���ߴ�.

������ �������� Send ��û �� GSendBufferManager->Open(reserveSize)�� ���ؼ�
TLS�� SendBufferChunk�� reserveSize��ŭ�� �Ҵ�޴´�.
�׸��� �� Chunk�� buffer �ּҿ� writeSize��ŭ�� ǥ���ϴ� SendBufferRef�� ����
owner�� Chunk�� �Ѵ�.
�� ���� Chunk�� Ref Count�� 2( TLS���� +1, SendBufferRef�� owner +1)�� �ȴ�.
���࿡ �� Chunk�� ���� �� ã�ٰ� �����ϰ� �ٽ� Send�� �ϰ� �Ǹ� 
SendBufferManager������ Chunk�� Pool���� ���� ����ų� �ִ� ���� �޾ƿ´�.
�׷��� �� ������ ���ؿ� Send�� �ؼ� ����� Chunk�� Ref Count�� -1�� �Ǽ�
SendBufferRef�� �� Chunk�� ����Ű�� �ִ�.
�� ���� �ｼ SendBufferRef�� WSASend�� �Ϸ�ǰ� Iocp ť���� �ƾ �����尡 
ProcessSend�� ���ؼ� �� �ش��ϴ� Session�� ���� sendEvent�� SendBuffers�� Ŭ�����ϴ�
������ sendBufferRef�� �Ҹ�ȴ�.
���� �Ʊ� �� SendBufferRef�� ���� send�� �� �����Ͱ� �Ϸ�Ǳ� �������� 
Chunk�� Ref Count�� ���� �Ǹ鼭 ����ְ� , �ڵ������� send�� �Ϸ�� 
sendBufferRef���� �Ҹ�Ǹ鼭 Ref Count�� 0�� �Ǹ� �Ҹ�ȴ�.
*/