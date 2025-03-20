#pragma once

//	Multiple Write One Read
template<typename T>
class SwapQueue
{
	enum
	{
		WRITE_QUEUE_INDEX = 0,
		READ_QUEUE_INDEX = 1,
		MAX_QUEUE_COUNT = 2,
	};

public:
	SwapQueue()
	{
		_writeQueue = &_queue[WRITE_QUEUE_INDEX];
		_readQueue = &_queue[READ_QUEUE_INDEX];
	}

public:
	void Push(const T& item)
	{
		std::lock_guard<std::mutex> lockGuard(_lock);
		_writeQueue->push(item);
	}

	void PopAll(OUT std::queue<T>& items)
	{
		{
			std::lock_guard<std::mutex> lockGuard(_lock);

			if (_readQueue->empty() == true && _writeQueue->empty() == true)
				return;

			if (_readQueue->empty() == true)
				Swap();
		}

		while (_readQueue->empty() == false)
		{
			T item = _readQueue->front();
			items.push(item);
			_readQueue->pop();
		}
	}

	void Swap()
	{
		if (_writeQueue == &_queue[WRITE_QUEUE_INDEX])
		{
			_writeQueue = &_queue[READ_QUEUE_INDEX];
			_readQueue = &_queue[WRITE_QUEUE_INDEX];
		}
		else
		{
			_writeQueue = &_queue[WRITE_QUEUE_INDEX];
			_readQueue = &_queue[READ_QUEUE_INDEX];
		}
	}

private:
	//	TODO
	std::queue<T>	_queue[MAX_QUEUE_COUNT];
		
	std::queue<T>* _writeQueue = nullptr;
	std::queue<T>* _readQueue = nullptr;

	std::mutex _lock;
};

