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
	//	SendBufferChunk의 예약해 놓은 사이즈 보다 내가 데이터를 쓴 사이즈가 더 크다???
	ASSERT_CRASH(_reserveSize >= writeSize);
	_writeSize = writeSize;
	_owner->Close(writeSize);
}

//	TLS 영역 -> 멀티스레드 x
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
	//	SendBufferChunk를 통해 Send를 할려고 예약할 데이터의 사이즈가 
	//	Chunk 사이즈 보다 크다??
	ASSERT_CRASH(reserveSize <= SEND_BUFFER_CHUNK_SIZE);
	//	재오픈 체크
	ASSERT_CRASH(_isOpen == false);

	//	남은 사이즈보다 예약 사이즈가 더 크다??
	if (reserveSize > FreeSize())
		return nullptr;

	//	
	_isOpen = true;
	return ObjectPool<SendBuffer>::MakeShared(shared_from_this(), Buffer(), reserveSize);
}

void SendBufferChunk::Close(uint32 writeSize)
{
	ASSERT_CRASH(_isOpen == true);

	//	Open 완료
	_isOpen = false;

	//	Chunk의 usedSize에 데이터를 쓴 사이즈 추가
	_usedSize += writeSize;
}

bool SendBufferManager::_isSendBufferChunkPoolPushed = true;

SendBufferRef SendBufferManager::Open(uint32 size)
{
	//	스레드마다 고유한 SendBufferChunk가 없으면 SendBufferManager의 SendBufferChunk Pool에서 받아온다.
	if (LSendBufferChunk == nullptr)
	{
		LSendBufferChunk = Pop();
		LSendBufferChunk->Reset();
	}

	ASSERT_CRASH(LSendBufferChunk->IsOpen() == false);

	//	다 썻으면 새로운 SendBufferChunk로 교체
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

//	SendBufferChunk 재사용
void SendBufferManager::PushGlobal(SendBufferChunk* buffer)
{
	cout << "SendBufferChunk Pool Push" << endl;

	if (_isSendBufferChunkPoolPushed)
		GSendBufferManager->Push(SendBufferChunkRef(buffer, PushGlobal));
	else
		cdelete(buffer);
}

