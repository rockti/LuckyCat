#pragma once
#include "BaseDataType.h"

//最多80个字段
#define MAX_MEMB				80
#define MAX_FIELD_NAME_LEN		100

//每种数据类型的标示符
enum MemberType
{
	FT_BYTE,	//单字节	8位		无符号
	FT_WORD,	//双字节	16位	无符号	目前没有用到该类型
	FT_DWORD,	//四字节	32位	无符号	目前没有用到该类型
	FT_REAL4,
	FT_REAL8,	//八字节	64位	带符号双精度
	FT_LONG,	//四字节	32位	带符号
};

//类成员属性结构
struct TMemberDesc
{
	MemberType nType;			//成员类型
	char szName[30];			//成员名称
	int nSize;					//成员长度
	int nStructOffset;			//成员在类中的偏移量
	int nStreamOffset;			//成员在字节流中的偏移量
};
#define DEF_SETUP_MEMBER(type,id) 	void SetupMember(type&, int offset, char* name, int length)	{ SetupMember(id,offset,name,length); }

	
//域的描述类
class  CFieldDescribe  
{
public:
    typedef void (*describeFunc)(); //定义一个指针函数类型
	CFieldDescribe(WORD FieldID, int nStructSize, char const* pszFieldName, char const* pszComment, describeFunc func);
	~CFieldDescribe();
public:
	//填充类型INT1类型Id FT_BYTE的对照表的方法
	DEF_SETUP_MEMBER(CBaseCharType, FT_BYTE);
		
	//填充类型INT4类型Id FT_WORD的对照表的方法
	DEF_SETUP_MEMBER(CBaseIntType, FT_DWORD);

	//填充类型INT8类型Id FT_WORD的对照表的方法
	DEF_SETUP_MEMBER(CBaseLongType, FT_LONG);
				
	//填充类型INT8类型Id FT_REAL8的对照表的方法
	DEF_SETUP_MEMBER(CBaseFloatType, FT_REAL8);
	
	//填充类型BYTEARRAY类型Id FT_BYTE的对照表的方法
	DEF_SETUP_MEMBER(CBaseStringType, FT_BYTE);

	//将对象转换为字节流	@remark 字节流中的成员变量是高位在前
	void StructToStream(char* pStruct, char* pStream, bool bInSys=false);
	//将字节流转换为对象
	void StreamToStruct(char* pStruct, char* pStream, bool bInSys=false);	
	//获取域大小
	int GetStructSize()	{ return m_nStructSize; }	
	//获取字节流长度
	int GetStreamSize()	{ return m_nStreamSize; }
	//类成员个数
	int GetMemberCount()	{ return m_nTotalMember; }	
	//获取域中每个成员的描述
	TMemberDesc* GetMemberDesc(int index)	{ return &m_MemberDesc[index]; }
	//获取域的名称
	char const* GetFieldName()	{ return m_szFieldName; }

private:
	//填充类型和类型Id对照表
	inline void SetupMember(MemberType nType, int nStructOffset, char* szName, int nSize)
	{
		TMemberDesc* pDesc = &m_MemberDesc[m_nTotalMember];
		pDesc->nType = nType;					//成员类型
		strcpy(pDesc->szName,szName);			//名称
		m_nStreamSize += nSize;
		pDesc->nSize = nSize;					//长度
		pDesc->nStructOffset = nStructOffset;	//类偏移量
		pDesc->nStreamOffset = m_nStreamSize;	//流偏移量
		m_nTotalMember++;
	}

public:
	WORD m_FieldID;							//域ID
	char m_szComment[MAX_FIELD_NAME_LEN];	//域注释
	int m_nStructSize;						//域大小
	int m_nStreamSize;						//域转换成的字节流长度

private:
	int m_nTotalMember;						//域成员个数
	char m_szFieldName[MAX_FIELD_NAME_LEN];	//域名
	TMemberDesc m_MemberDesc[MAX_MEMB];		//域成员属性数组
};

#define TYPE_DESCRIPTOR(members) void DescribeMembers() { members; } static CFieldDescribe m_Describe 
#define TYPE_DESC(member) m_Describe.SetupMember(member,int((char*)&member-(char*)this),#member,sizeof(member))
#define REGISTER_FIELD(fid,field, comment)   static void DescribeMemberOf##field() { field f; f.DescribeMembers(); } CFieldDescribe field::m_Describe(fid,sizeof(field),#field,comment,&DescribeMemberOf##field)

