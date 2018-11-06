#pragma once

#include "mainHeader.hpp"

class threadController
{
private:
	thread* _threadArray;
	int _activeThreadCounter, _allThreadCounter;
	vector<int> _availableThreads, _usingThreads;

public:
	threadController(int allThreadCounter)
	{
		_allThreadCounter = allThreadCounter;
		_threadArray = new thread[_allThreadCounter];

		for (size_t i = 0; i < _allThreadCounter; i++)
		{
			_availableThreads.push_back(i);
		}
	}

	template<typename fnType, typename... Args>
	int setThread(fnType (*function)(Args...), Args... args)
	{
		for (auto i = _usingThreads.begin(); i < _usingThreads.end(); i++)
		{
			if (_threadArray[*i].joinable() == false)
			{
				_availableThreads.push_back(*i);
				_usingThreads.erase(i);
			}
		}

		if (_availableThreads.size() == 0) 
		{
			return -1;
		}

		int threadNumber = _availableThreads.back();
		_availableThreads.pop_back();
		_usingThreads.push_back(threadNumber);

		_threadArray[threadNumber] = thread(function, args...);
	}

	~threadController()
	{
		for (auto i = _usingThreads.begin(); i < _usingThreads.end(); i++)
			_threadArray[*i].detach();

		delete[] _threadArray;
	}
};