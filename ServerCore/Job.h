#pragma once
#include <functional>

using CallbackType = std::function<void()>;

class Job
{
public:
	Job(CallbackType&& callback)
		: _callback(std::move(callback))
	{

	}
	template<typename T, typename Ret, typename... Args>
	Job(shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args)
	{
		//	Ref Count ����
		_callback = [owner, memFunc, args...]()
		{
			(owner.get()->*memFunc)(args...);
		};
	}

public:
	void Execute()
	{
		_callback();
	}

private:
	CallbackType _callback;
};
