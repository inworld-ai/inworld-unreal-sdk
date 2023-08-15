/**
 * Copyright 2022 Theai, Inc. (DBA Inworld)
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace Inworld
{
	template <typename T>
	class SharedQueue
	{
	public:
		SharedQueue();
		~SharedQueue();

		T& Front();
		void PopFront();
		bool PopFront(T& Item);

		void PushBack(const T& Item);
		void PushBack(T&& Item);

		int Size();
		bool IsEmpty();

	private:
		std::deque<T> _Queue;
		std::mutex _Mutex;
	};

	template <typename T>
	SharedQueue<T>::SharedQueue() {}

	template <typename T>
	SharedQueue<T>::~SharedQueue() {}

	template <typename T>
	T& SharedQueue<T>::Front()
	{
		std::unique_lock<std::mutex> Lock(_Mutex);
		return _Queue.front();
	}

	template <typename T>
	void SharedQueue<T>::PopFront()
	{
		std::unique_lock<std::mutex> Lock(_Mutex);
		_Queue.pop_front();
	}

	template <typename T>
	bool Inworld::SharedQueue<T>::PopFront(T& Item)
	{
		std::unique_lock<std::mutex> Lock(_Mutex);
		if (!_Queue.empty())
		{
			Item = _Queue.front();
			_Queue.pop_front();
			return true;
		}
		return false;
	}

	template <typename T>
	void SharedQueue<T>::PushBack(const T& Item)
	{
		std::unique_lock<std::mutex> Lock(_Mutex);
		_Queue.push_back(Item);
		Lock.unlock();
	}

	template <typename T>
	void SharedQueue<T>::PushBack(T&& Item)
	{
		std::unique_lock<std::mutex> Lock(_Mutex);
		_Queue.push_back(std::move(Item));

	}

	template <typename T>
	int SharedQueue<T>::Size()
	{
		std::unique_lock<std::mutex> Lock(_Mutex);
		int Size = _Queue.size();
		return Size;
	}

	template <typename T>
	bool Inworld::SharedQueue<T>::IsEmpty()
	{
		return Size() == 0;
	}
}