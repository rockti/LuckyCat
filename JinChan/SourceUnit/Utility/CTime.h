#pragma once

//CTime是表示绝对时间（时刻）的类，精确到秒 它以整数的形式存贮当天的秒数，可以计算用于时间的计算
class CTime
{
public :
	enum	{ INVALID_TIME = 0xffffffff, MAX_TIME_VALUE = 24*3600 };

	//构造函数用字符串表示的时刻，格式为HH24:MM:SS
	CTime(char const* pszTime);

	//构造函数用整数表示的时刻，即从零点起的秒数
	CTime(DWORD lTime);
	~CTime();

	
	DWORD ToLong()	{ return m_lTimeValue; }						//取整数表示的时刻
	bool IsValid()	{ return m_lTimeValue != INVALID_TIME; }		//判断本对象是否表示了一个合法的时刻
	char const* ToString()	{ return LongToTime(m_lTimeValue); }	//取用字符串表示的时刻

	int Hour()	{ return m_lTimeValue/3600; }
	int Minute()	{ return (m_lTimeValue%3600)/60; }
	int Second()	{ return m_lTimeValue%60; }

	//本时刻加上一定的时间	超过24点会重新从0点开始 
	CTime operator+(DWORD const lTimeSpan);
	
	//本时刻减去一定的时间	小于0点会重新开始 
	CTime operator-(DWORD const lTimeSpan);

	//计算两个时刻所差的秒数
	DWORD operator-(CTime const& t);

	//判断两个时刻是否相等
	bool operator==(CTime const& t);
	bool operator==(char const* pszTime);
	
	//检查一个字符串是否表示了合法的时刻，即符合"HH24:MM:SS"格式
	static bool IsValid(char const* pszTime)	{ return TimeToLong(pszTime) != INVALID_TIME; }
	
	//将字符串表示的时刻转换为整数表示的时刻
	static DWORD TimeToLong(char const* pszTime);

	//将整数表示的时刻转换为字符串表示的时刻
	static char const* LongToTime(DWORD lTime);

private:
	DWORD  m_lTimeValue;			/**<用整数表示的时刻 */
};


//CDate是表示日期的类
class CDate
{
public :
	//构造函数@param pszDate 字符串表示的日期，格式为"YYYYMMDD"
	CDate(const char *pszDate);
	//构造函数@param lDate 整数表示的日期（1980-1-1至今的天数）
	CDate(DWORD lDate);
	~CDate();

	//取字符串表示的日期
	char const* ToString()	{ 	return m_strDate.c_str(); }

	//将整数表示的日期转换为一个字符串表示的日期	@param lDate 要转换的整数表示的日期	@return 转换出的字符串表示的日期
	static char* LongToDate(DWORD lDate);
	//将一个字符串转换为整数表示的日期	@param pszDate 要转换的字符串	@return 转换出的整数表示的日期
	static DWORD DateToLong(char const* pszDate);

	//判断某一年是否为闰年	@param nYear 年份	@return 1 该年份为闰年，0 该年份不是闰年
	static int IsLeapYear(int nYear);
	//取指定月份的天数
	static WORD GetDays(int nYear,int nMonth);
	//取指定日期是星期几	@param pszDate 指定的日期	@return 整数表示的星期几（1-7表示星期一至星期天）
	static int GetWeekDay(const char *pszDate);
	//判断一个字符串是否合法地表示日期("YYYYMMDD")	@param pszData	@return true 合法的日期表示，false 非法的日期表示
	static bool IsValid(const char *pszDate);

	//取整数表示的日期
	DWORD ToLong();

	//取整数表示的月中的日期@return 整数表示的月中的日期（1-31）
	int Day();
	//取整数表示的月份		@return 整数表示的月份（1-12）
	int Month();
	//取整数表示的年份		@return 整数表示的年份（从公元开始）
	int Year();
	//取整数表示的星期几	@return 整数表示的星期几（1-7表示星期一至星期天）
	int WeekDay();

	//加指定天数	@param i 加的天数	@return 加后的结果
	CDate operator+(DWORD i);
	//CDate对象代表的日期增加一天	@return 增加后的对象
	CDate &operator++(int);
	//CDate对象代表的日期增加指定天数	@param i 加的天数	@return 增加后的对象
	CDate &operator +=(DWORD i);

	//加指定天数	@return 加后的结果(整数表示）
	DWORD operator-(CDate &date);
	//减指定天数	@return 减后的结果(整数表示）
	DWORD operator-(const char* pszDate);
	//CDate对象代表的日期减少指定天数	@param i 减的天数	@return 减少后的对象
	CDate &operator -=(DWORD i);
	//CDate对象代表的日期减少一天	@return 减少后的对象
	CDate &operator--(int);
	//减指定天数	@param i 减的天数	@return 减后的结果
	CDate operator-(DWORD i);
	
	//判断本对象的日期与另一日期是否相等@param date 要比对的另一日期 @return true 相等，false不相等
	bool operator==(CDate &date);
	//判断本对象的日期与另一字符串表示的日期是否相等	@param pszData 要比对的字符串表示的另一日期	@return true 相等，false不相等
	bool operator==(const char *pszDate);	

private:
	string m_strDate;		/**<用字符串表示的日期 */
};
