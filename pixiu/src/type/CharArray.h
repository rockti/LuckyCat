#pragma once
#include <string>
#include <algorithm>

#ifdef max 
#undef max
#endif

template<int size>
class CharArray {
public:
	static_assert(size < 128);
	CharArray() { memset(m_array, 0, size); };
	explicit CharArray(char const* str) {
		memset(m_array, 0, size);
		strcpy_s(m_array, str);
	}
	explicit CharArray(std::string const& other) {
		memset(m_array, 0, size);
		strcpy_s(m_array, other.c_str());
	}
	CharArray(CharArray const& other) {
		memcpy(m_array, other.m_array, std::max(size, other.Size()));
	}
	inline char const* Str() const { return m_array; }
	inline char at(size_t index) const { return m_array[index]; }
	inline size_t Length() const { return strlen(m_array); }
	CharArray& operator = (char const* other) {
		strcpy_s(m_array, other);
		return *this;
	}

	CharArray& operator = (CharArray const& other) {
		memcpy(m_array, other.m_array, std::max(size, other.Size()));
		return *this;
	}
	inline int Size() const { return size; }
private:
	char m_array[size];
};