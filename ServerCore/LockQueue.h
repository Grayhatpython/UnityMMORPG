#pragma once

template<typename T>
class LockQueue
{
public:
	void Push(T item)
	{
		WRITE_LOCK;
		_items.push(item);
	}

	T Pop()
	{
		WRITE_LOCK;
		//	TEMP : ����Ʈ �������� ��� ����
		if (_items.empty())
			return T();

		auto item = _items.front();
		_items.pop();
		return item;
	}

	T LocklessPop()
	{
		//	TEMP : ����Ʈ �������� ��� ����
		if (_items.empty())
			return T();

		auto item = _items.front();
		_items.pop();
		return item;
	}

	void PopAll(OUT Vector<T>& items)
	{
		WRITE_LOCK;
		while (T item = LocklessPop())
			items.push_back(item);
	}
	void Clear()
	{
		WRITE_LOCK;
		_items = Queue<T>();
	}

private:
	USE_LOCK;
	Queue<T> _items;
};