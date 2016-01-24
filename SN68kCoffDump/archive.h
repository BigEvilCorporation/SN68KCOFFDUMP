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

	enum SeekBase
	{
		SEEK_START,
		SEEK_CURRENT
	};

	Stream(char* ptr)
	{
		//TODO: Output support
		m_direction = STREAM_IN;
		m_start = ptr;
		m_ptr = ptr;
	}

	Direction GetDirection() const { return m_direction; }

	s64 Seek(s64 offset, SeekBase base)
	{
		if(base == SEEK_START)
			m_ptr = (char*)(m_start + offset);
		else if(base == SEEK_CURRENT)
			m_ptr += offset;

		return (s64)(m_ptr - m_start);
	}

	template <typename T> void Serialise(T& value)
	{
		value.Serialise(*this);
	}

	void Serialise(u8& value)
	{
		value = *(u8*)m_ptr;
		m_ptr += sizeof(u8);
	}

	void Serialise(s8& value)
	{
		value = *(s8*)m_ptr;
		m_ptr += sizeof(s8);
	}

	void Serialise(u16& value)
	{
		value = *(u16*)m_ptr;
		m_ptr += sizeof(u16);
	}

	void Serialise(s16& value)
	{
		value = *(s16*)m_ptr;
		m_ptr += sizeof(s16);
	}

	void Serialise(u32& value)
	{
		value = *(u32*)m_ptr;
		m_ptr += sizeof(u32);
	}

	void Serialise(s32& value)
	{
		value = *(s32*)m_ptr;
		m_ptr += sizeof(s32);
	}

	void Serialise(std::string& value)
	{
		u8 length;
		Serialise(length);
		value.resize(length);
		memcpy(&value[0], m_ptr, length);
		m_ptr += length;
	}

	void Serialise(std::string& value, u32 length)
	{
		value.resize(length + 1);
		memcpy(&value[0], m_ptr, length);
		value[length] = 0;
		m_ptr += length;
	}

	void Serialise(u8* value, u32 length)
	{
		memcpy(value, m_ptr, length);
		m_ptr += length;
	}

private:
	const char* m_start;
	char* m_ptr;
	Direction m_direction;
};