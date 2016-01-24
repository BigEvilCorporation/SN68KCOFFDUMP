#pragma once

#include <string>

#include "atoms.h"

class Stream
{
public:
	enum Direction
	{
		STREAM_OUT,
		STREAM_IN
	};

	Stream(char* ptr)
	{
		//TODO: Output support
		m_direction = STREAM_IN;
		m_ptr = ptr;
	}

	Direction GetDirection() const { return m_direction; }

	template <typename T> void Serialise(T& value)
	{
		value.Serialise(*this);
	}

	void Serialise(u8& value)
	{
		value = *(u8*)m_ptr;
		m_ptr += sizeof(u8);
	}

	void Serialise(u16& value)
	{
		value = *(u16*)m_ptr;
		m_ptr += sizeof(u16);
	}

	void Serialise(u32& value)
	{
		value = *(u32*)m_ptr;
		m_ptr += sizeof(u32);
	}

	void Serialise(std::string& value)
	{
		u8 length;
		Serialise(length);
		value.resize(length);
		memcpy(&value[0], m_ptr, length);
		m_ptr += length;
	}

	void Serialise(std::string& value, u8 length)
	{
		value.resize(length + 1);
		memcpy(&value[0], m_ptr, length);
		value[length] = 0;
		m_ptr += length;
	}

private:
	char* m_ptr;
	Direction m_direction;
};