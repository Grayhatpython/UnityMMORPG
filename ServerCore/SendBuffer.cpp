#include "pch.h"
#include "SendBuffer.h"

SendBuffer::SendBuffer(SendBufferChunkRef owner, BYTE* buffer, uint32 reserveSize)
	: _owner(owner), _buffer(buffer), _reserveSize(reserveSize)
{

}

SendBuffer::~SendBuffer()
{
	//cout << "~SendBuffer" << endl;
}

void SendBuffer::Close(uint32 writeSize)
{
	//	SendBufferChunk�� ������ ���� ������ ���� ���� �����͸� �� ����� �� ũ��???
	ASSERT_CRASH(_reserveSize >= writeSize);
	_writeSize = writeSize;
	_owner->Close(writeSize);
}

//	TLS ���� -> ��Ƽ������ x
SendBufferChunk::SendBufferChunk()
{
}

SendBufferChunk::~SendBufferChunk()
{
}

void SendBufferChunk::Reset()
{
	_isOpen = false;
	_usedSize = 0;
}

SendBufferRef SendBufferChunk::Open(uint32 reserveSize)
{
	//	SendBufferChunk�� ���� Send�� �ҷ��� ������ �������� ����� 
	//	Chunk ������ ���� ũ��??
	ASSERT_CRASH(reserveSize <= SEND_BUFFER_CHUNK_SIZE);
	//	����� üũ
	ASSERT_CRASH(_isOpen == false);

	//	���� ������� ���� ����� �� ũ��??
	if (reserveSize > FreeSize())
		return nullptr;

	//	
	_isOpen = true;
	return ObjectPool<SendBuffer>::MakeShared(shared_from_this(), Buffer(), reserveSize);
}

void SendBufferChunk::Close(uint32 writeSize)
{
	ASSERT_CRASH(_isOpen == true);

	//	Open �Ϸ�
	_isOpen = false;

	//	Chunk�� usedSize�� �����͸� �� ������ �߰�
	_usedSize += writeSize;
}

bool SendBufferManager::_isSendBufferChunkPoolPushed = true;

SendBufferRef SendBufferManager::Open(uint32 size)
{
	//	�����帶�� ������ SendBufferChunk�� ������ SendBufferManager�� SendBufferChunk Pool���� �޾ƿ´�.
	if (LSendBufferChunk == nullptr)
	{
		LSendBufferChunk = Pop();
		LSendBufferChunk->Reset();
	}

	ASSERT_CRASH(LSendBufferChunk->IsOpen() == false);

	//	�� ������ ���ο� SendBufferChunk�� ��ü
	if (LSendBufferChunk->FreeSize() < size)
	{
		LSendBufferChunk = Pop();
		LSendBufferChunk->Reset();
	}

	auto freeSize = LSendBufferChunk->FreeSize();

	return LSendBufferChunk->Open(size);
}

SendBufferChunkRef SendBufferManager::Pop()
{
	{
		WRITE_LOCK;
		if (_sendBufferChunks.empty() == false)
		{
			SendBufferChunkRef sendBufferChunk = _sendBufferChunks.back();
			_sendBufferChunks.pop_back();
			return sendBufferChunk;
		}
	}

	return SendBufferChunkRef(cnew<SendBufferChunk>(), PushGlobal);
}

void SendBufferManager::Push(SendBufferChunkRef buffer)
{
	WRITE_LOCK;
	_sendBufferChunks.push_back(buffer);
}

//	SendBufferChunk ����
void SendBufferManager::PushGlobal(SendBufferChunk* buffer)
{
	cout << "SendBufferChunk Pool Push" << endl;

	if (_isSendBufferChunkPoolPushed)
		GSendBufferManager->Push(SendBufferChunkRef(buffer, PushGlobal));
	else
		cdelete(buffer);
}

