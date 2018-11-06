#pragma once

#include "mainHeader.hpp"

using namespace std;

class Data
{
private:
	string _buffer;
	char* _c_buffer;
	int _c_buffer_len;
	bool _setted = false;
public:
	Data()
	{
	}

	template <typename T>
	Data(T data)
	{
		_buffer = to_string(data);
		_c_buffer_len = 0;
	}

	Data(string data)
	{
		_buffer = data;
		_c_buffer_len = 0;
	}

	Data(const char* data)
	{
		_c_buffer = createBuffer(BLOCK_SIZE);

		memcpy(_c_buffer, data, BLOCK_SIZE);
	}

	Data(char* data)
	{
		_c_buffer = createBuffer(BLOCK_SIZE);

		memcpy(_c_buffer, data, BLOCK_SIZE);
	}

	Data(const Data& data)
	{
		_buffer = data._buffer;
		_c_buffer_len = 0;

		if (data._c_buffer_len != 0)
		{
			createBuffer(data._c_buffer_len);
			_c_buffer_len = data._c_buffer_len;

			for (int i = 0; i < data._c_buffer_len; ++i)
			{
				_c_buffer[i] = data._c_buffer[i];
			}
		}
	}

	Data& operator=(Data& data)
	{
		_buffer = data._buffer;
		_c_buffer_len = 0;

		if (data._c_buffer_len != 0)
		{
			createBuffer(data._c_buffer_len);
			_c_buffer_len = data._c_buffer_len;

			for (int i = 0; i < data._c_buffer_len; i++)
			{
				_c_buffer[i] = data._c_buffer[i];
			}
		}

		return *this;
	}

	Data& operator=(char* data)
	{
		_buffer = data;
		return *this;
	}

	Data& operator=(const char* data)
	{
		_buffer = data;
		return *this;
	}

	template <typename T>
	Data& operator=(T data)
	{
		_buffer = to_string(data);
		return *this;
	}

	string getDataSize_str()
	{
		return to_string(_buffer.size());
	}

	const char* getCharString()
	{
		if (_buffer.empty())
		{
			return _c_buffer;
		}

		return _buffer.c_str();
	}

	int getDataSize()
	{
		if (_buffer.empty())
		{
			return BLOCK_SIZE;
		}

		return (int)_buffer.size();
	}

	string getString()
	{
		return _buffer;
	}

	char* createBuffer(int len)
	{
		if (_setted == true)
		{
			delete _c_buffer;
		}

		_c_buffer_len = len;

		try
		{
			_c_buffer = new char[len + 1];
		}
		catch (bad_alloc& ba)
		{
			throw runtime_error(ba.what());
		}

		_c_buffer[len] = '\0';
		_setted = true;
		return _c_buffer;
	}

	char* getBuffer()
	{
		return _c_buffer;
	}

	~Data()
	{
		if (_setted == true)
		{
			delete _c_buffer;
		}
	}
};
