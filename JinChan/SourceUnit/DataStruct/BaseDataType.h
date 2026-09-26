#pragma once
//定义了内部数据的底层结构

//定义一个很小的浮点数，这个数应当不影响计算结果，又大于各类浮点计算误差
#define TINY_DOUBLE 0.0000001
#define EnumNull ' '

#include "type.hpp"

class CBaseCharType
{
};
class CBaseIntType
{
};
class CBaseLongType
{
};
class CBaseFloatType
{
};
class CBaseStringType
{
};

//CCharType(BYTE单字节)
class CCharType : public CBaseCharType
{
public:
	CCharType(void)	{ clear(); }
	CCharType(CCharType const& c)	{ setValue(c.getValue()); }
	CCharType(char const c)	{ setValue(c); }
	CCharType const& operator = (CCharType const& c)
	{
		setValue(c.getValue());
		return c;
	}
	char const operator = (char const c)
	{
		setValue(c);
		return c;
	}
	char const* operator = (char const* v)
	{
		setValue(*v);
		return v;
	}

	bool isNull(void) const { return false; }
	bool isValid(void) const { return true; }
	void clear(void)	{ value = '\0'; }
	char getValue(void) const	{ return value; }
	void setValue(const char v) { value = v; }
	void getString(char* strBuf) const
	{
		if (NULL != strBuf)
		{
			sprintf(strBuf, "%c", value);
		}	
	}	
	operator const char (void) const	{ return value; }
	unsigned int hash(unsigned int seed) const
	{
		unsigned long ret = seed;
		unsigned long v = 0x100 | value;
		int r = (int)((v>>2)^v)&0x0f;
		ret = (ret<<r)|(ret>>(32-r));
		ret &= 0xFFFFFFFFL;
		ret ^= v*v;
		return ((ret>>16)^ret);
	}
protected:
	char value;
};
typedef const CCharType CReadOnlyCharType;

//CIntType(无符号四字节)
class CIntType : public CBaseIntType
{
public:
	CIntType(void)	{ clear(); }
	CIntType(CIntType const& i)	{ setValue(i.getValue()); }
	CIntType(uint32_t const i)	{ setValue(i); }
	CIntType const& operator = (CIntType const& i)	
	{
		setValue(i.getValue());
		return i;
	}
	uint32_t const operator = (uint32_t const i)
	{
		setValue(i);
		return i;
	}
// 	char const* operator = (const char* v)
// 	{
// 		setValue(atoi(v));
// 		return v;
// 	}
	bool isNull(void) const	{ return false; }
	bool isValid(void) const	{ return true; }
	void clear(void)	{ value = 0; }
	int32_t const getValue(void) const	{ return value; }
	void setValue(const int32_t v)	{ value = v; }
	void getString(char* strBuf) const
	{
		if (NULL != strBuf)
		{
			sprintf(strBuf, "%d", value);
		}	
	}	
	operator const uint32_t(void) const	{ return value; }
	uint32_t const operator += (const uint32_t v)
	{
		setValue(value+v);
		return value;
	}	
	uint32_t const operator -= (const uint32_t v)
	{
		setValue(value-v);
		return value;
	}
	uint32_t const operator *= (const uint32_t v)
	{
		setValue(value*v);
		return value;
	}
	uint32_t const operator /= (const uint32_t v)
	{
		setValue(value/v);
		return value;
	}
	uint32_t const operator %= (const uint32_t v)
	{
		setValue(value%v);
		return value;
	}

	uint32_t hash(uint32_t seed) const
	{
		uint32_t ret = seed;
		uint32_t v = 0x100 | value;
		uint32_t r = (uint32_t)((v>>2)^v)&0x0f;
		ret = (ret<<r)|(ret>>(32-r));
		ret &= 0xFFFFFFFFL;
		ret ^= v*v;
		return ((ret>>16)^ret);
	}

protected:
	uint32_t value;
};
typedef const CIntType CReadOnlyIntType;
#define DefineIntType(pureType, newType)			\
	typedef C##pureType C##newType;					\
	typedef CReadOnly##pureType CReadOnly## newType;

//CLongType(有符号八字节)、
class CLongType : public CBaseLongType
{
public:
	CLongType(void)	{ clear(); }
	CLongType(CLongType const& i)	{ setValue(i.getValue()); }
	CLongType(int32_t const i)	{ setValue(i); }
	template<class Base,class Tag>
	CLongType(NumberType<Base, Tag> const& other) {
		setValue(other.value());
	}
	CLongType const& operator = (CLongType const& i)
	{
		setValue(i.getValue());
		return i;
	}
	int32_t const operator = (int32_t const i)
	{
		setValue(i);
		return i;
	}

	template<class Base, class Tag>
	Base const operator = (NumberType<Base, Tag> const i) {
		setValue(i.value());
		return i.value();
	}
// 	char const* operator = (const char* v)
// 	{
// 		setValue(atol(v));
// 		return v;
// 	}

	bool isNull(void) const	{ return false; }
	bool isValid(void) const	{ return true; }
	void clear(void)	{ value = 0; }
	int32_t const getValue(void) const	{ return value; }
	void setValue(int32_t const v)	{ value = v; }
	void getString(char* strBuf) const
	{
		if (NULL != strBuf)
		{
			sprintf(strBuf, "%d", value);
		}	
	}	

	operator int32_t const (void) const	{ return value; }
	int32_t const operator += (const int32_t v)
	{
		setValue(value+v);
		return value;
	}
	int32_t const operator -= (const int32_t v)
	{
		setValue(value-v);
		return value;
	}
	int32_t const operator *= (const int32_t v)
	{
		setValue(value*v);
		return value;
	}
	int32_t const operator /= (const int32_t v)
	{
		setValue(value/v);
		return value;
	}
	int32_t const operator %= (const int32_t v)
	{
		setValue(value%v);
		return value;
	}

	uint32_t hash(uint32_t seed) const
	{
		uint32_t ret = seed;
		uint32_t v = 0x100 | value;
		uint32_t r = (uint32_t)((v>>2)^v)&0x0f;
		ret = (ret<<r)|(ret>>(32-r));
		ret &= 0xFFFFFFFFL;
		ret ^= v*v;
		return ((ret>>16)^ret);
	}

protected:
	int32_t value;
};
typedef const CLongType CReadOnlyLongType;

//CRangeIntType<int from, int to>是一个通过制定范围存储一个整数的模板类，但是在设置值时将做检测
template <int32_t from, int32_t to> class CRangeIntType : public CBaseIntType
{
public:
	CRangeIntType(void)	{ clear(); }
	CRangeIntType(const CRangeIntType<from,to>& i)	{ setValue(i.getValue()); }
	CRangeIntType(const int32_t i)	{ setValue(i); }
	CRangeIntType<from,to> const& operator = (const CRangeIntType<from,to>& i)
	{
		setValue(i.getValue());
		return i;
	}

	int const operator = (const int32_t i)
	{
		setValue(i);
		return i;
	}
	char const* operator = (const char* v)
	{
		setValue(atoi(v));
		return v;
	}

	void clear(void)	{ value = from; }
	int32_t const getValue(void) const	{ return value; }
	void setValue(const int32_t v)	{ value = v; }
	void getString(char* strBuf) const
	{
		if (NULL != strBuf)
		{
			sprintf(strBuf,"%d", value);
		}	
	}
	bool isNull(void) const	{ return false; }
	bool isValid(void) const
	{
		if ((value>to) || (value<from))
			return false;
		else
			return true;
	}

	operator const int32_t(void) const	{ return value; }

	int32_t const operator += (const int32_t v)
	{
		setValue(value+v);
		return value;
	}
	int32_t const operator -= (const int32_t v)
	{
		setValue(value-v);
		return value;
	}
	int32_t const operator *= (const int32_t v)
	{
		setValue(value*v);
		return value;
	}
	int32_t const operator /= (const int32_t v)
	{
		setValue(value/v);
		return value;
	}
	int32_t const operator %= (const int32_t v)
	{
		setValue(value%v);
		return value;
	}

	uint32_t hash(uint32_t seed) const
	{
		unsigned long ret = seed;
		unsigned  long v = 0x100 | value;
		uint32_t r = (uint32_t)((v>>2)^v)&0x0f;
		ret = (ret<<r)|(ret>>(32-r));
		ret &= 0xFFFFFFFFL;
		ret ^= v*v;
		return ((ret>>16)^ret);
	}
protected:
	///存储整数的值
	int32_t value;
};

//CFloatType是一个内部数据的浮点数类
template <int length, int precision> 
class CDoubleType : public CBaseFloatType
{
public:
	CDoubleType(void)	{ clear(); }
	CDoubleType(CDoubleType const& f)	{ setValue(f.getValue()); }
	template<class Tag>
	CDoubleType(DoubleType<Tag> const& f) { setValue(f.value()); }
	CDoubleType(double const value)	{ setValue(value); }
	CDoubleType const& operator = (CDoubleType const& f)
	{
		setValue(f.getValue());
		return f;
	}
	double const operator = (double const value)
	{
		setValue(value);
		return value;
	}
	template<class Tag>
	double const operator = (DoubleType<Tag> const value) {
		setValue(value.value());
		return value.value();
	}
	char const* operator = (char const* v)
	{
		if (v[0] == '\0')
		{
			setValue(DBL_MAX);
		}
		else
		{
			setValue((double)atof(v));
		}
		return v;
	}
	void clear(void)	{ value = 0.0; }
	double const getValue(void) const	{ return value; }
	void setValue(const double v)
	{
		if ((v<TINY_DOUBLE)&&(v>-TINY_DOUBLE))
		{
			value = 0.0;
		}
		else
		{
			value = v;
		}
	}
	void getString(char* strBuf) const
	{
		if (strBuf != NULL)
		{
			if (value== DBL_MAX)
			{
				strcpy(strBuf, "");
			}
			else
			{
				sprintf(strBuf, "%.6f", value);
			}
		}	
	}	
	bool isNull(void) const
	{
		if (DBL_MAX == value)
			return true;
		else
			return false;
	}
	bool isValid(void) const
	{
		if (isnan(value))
			return false;
		if (value == DBL_MAX)
			return true;	
		else if (value >= 91000000000)
			return false;
		else if (value <= -10000000)
			return false;
		return true;
	}
	operator const double (void) const	{ return value; }

	double const operator += (const double v)
	{
		setValue(value+v);
		return v;
	}
	double const operator -= (double v)
	{
		setValue(value-v);
		return v;
	}
	double const operator *= (const double v)
	{
		setValue(value*v);
		return v;
	}
	double const operator /= (const double v)
	{
		setValue(value/v);
		return v;
	}

	//整除某个值
	int const operator& (double const v) const
	{
		return (int)(floor(value/v+TINY_DOUBLE));
	}

	bool operator < (double r) const	{ return value < r-TINY_DOUBLE; }
	bool operator > (double r) const	{ return value > r+TINY_DOUBLE; }
	bool operator <= (double r) const	{ return value <= r+TINY_DOUBLE; }
	bool operator >= (double r) const	{ return value >= r-TINY_DOUBLE; }
	bool operator == (double r) const	{ return fabs(value-r)<=TINY_DOUBLE; }
	bool operator != (double r) const	{ return fabs(value-r)>TINY_DOUBLE; }
	bool operator < (int r) const	{ return value < r-TINY_DOUBLE; }
	bool operator > (int r) const	{ return value > r+TINY_DOUBLE; }
	bool operator <= (int r) const	{ return value <= r+TINY_DOUBLE; }
	bool operator >= (int r) const	{ return value >= r-TINY_DOUBLE; }
	bool operator == (int r) const	{ return fabs(value-r)<=TINY_DOUBLE; }
	bool operator != (int r) const	{ return fabs(value-r)>TINY_DOUBLE; }

	unsigned int hash(unsigned int seed) const
	{
		unsigned long ret = seed;
		const char* str = (char*)&value;
		long n = 0x100;
		for (int i=0; i<sizeof(double); i++)
		{
			unsigned long v = n|(*str);
			n += 0x100;
			int r = (int)((v>>2)^v)&0x0f;
			ret = (ret<<r)|(ret>>(32-r));
			ret &= 0xFFFFFFFFL;
			ret ^= v*v;
			str++;
		}
		return ((ret>>16)^ret);
	}

protected:
	double value;
};

//CStringType<int length>是一个根据字符串长度，直接保存字符串值的模板类，左对齐，最后加'\0'。
template <int length> 
class CStringType : public CBaseStringType
{
public:
	CStringType(void)	{ clear(); }
	CStringType(const CStringType<length>& s)	{ setValue(s.getValue()); }
	template<uint8_t size, class Tag>
	CStringType(const StrongCharArray<size,Tag>& s)	{ setValue(s.value()); }
	CStringType(const char* p)	{ setValue(p); }
	CStringType const& operator = (const CStringType<length>& s)
	{
		setValue(s.getValue());
		return s;
	}

	//等于操作符，从字符串中获取
	char const* operator = (const char* p)
	{
		setValue(p);
		return p;
	}
	template<uint8_t size, class Tag>
	char const* operator = (StrongCharArray<size, Tag> const& p) {
		setValue(p.value());
		return getValue();
	}
	//等于操作符，从整数中获取
	int const operator = (const int v)
	{
		char tmp[100];
		sprintf(tmp, "%*d", length, v);
		setValue(tmp);
		return v;
	}
	//等于操作符，从64位长整数中获取
	const long operator =(const long v)
	{
		char tmp[100];
		sprintf(tmp, "%*ld", length, v);
		setValue(tmp);
		return v;
	}

	//转化为字符串
	operator char const* (void) const	{ return buffer; }

	//判断是否为空
	bool isNull(void) const
	{
		char const* p = buffer;
		while (*p)
		{
			if (*p != ' ')
			{
				return false;
			}
			p++;
		}
		return true;
	}
	//判断是否合法
	bool isValid(void) const	{ return true; }
	//清除字符串内容
	void clear(void)	{ buffer[0]='\0'; }

	//设置字符串的值
	void setValue(char const* s)
	{
		if (NULL == s)
			buffer[0]='\0';
		else
		{
			strncpy(buffer, s, length);
			buffer[length]='\0';
		}
	}

	//在strBuf中得到本类型的字符串表示	
	void getString(char* strBuf) const
	{
		if (NULL != strBuf)
		{
			memcpy(strBuf, buffer,length);
			strBuf[length]='\0';
		}	
	}
	//获取字符串的值
	char const* getValue(void) const	{ return buffer; }

	//将右边的空格删除
	void trimRight(void)
	{
		char* p = buffer + strlen(buffer)-1;
		while (p >= buffer)
		{
			if (*p != ' ')
			{
				break;
			}
			*p = '\0';
			p--;
		}
	}

	//判断是否小于某个字符串
	bool operator < (char const* r) const	{ return strcmp(buffer, r) < 0; }

	//判断是否大于某个字符串
	bool operator > (char const* r) const	{ return strcmp(buffer, r) > 0; }

	//判断是否小于等于某个字符串
	bool operator <= (char const* r) const	{ return strcmp(buffer, r) <= 0; }

	//判断是否大于等于某个字符串
	bool operator >= (char const* r) const	{ return strcmp(buffer, r) >= 0; }

	//判断是否等于某个字符串
	bool operator == (const char* r) const	{ return strcmp(buffer, r) == 0; }

	//判断是否不等于某个字符串
	bool operator != (char const* r) const	{ return strcmp(buffer, r) != 0; }

	//产生hash值
	unsigned int hash(unsigned int seed) const
	{
		unsigned long ret = seed;
		char const* str = buffer;		
		if (*str == '\0')
		{
			return(ret);
		}
		long n = 0x100;
		while (*str)
		{
			unsigned long v = n|(*str);
			n += 0x100;
			int r = (int)((v>>2)^v)&0x0f;
			ret = (ret<<r)|(ret>>(32-r));
			ret &= 0xFFFFFFFFL;
			ret ^= v*v;
			str++;
		}
		return ((ret>>16)^ret);
	}

protected:
	///存储字符串的值
	char buffer[length+1];
};
#define DefineStringType(length, newType)				\
	typedef CStringType<length> C##newType;				\
	typedef const CStringType<length> CReadOnly##newType;
