#pragma once
#include "EntityCopy.h"
/*
template <class DesType, class SourceType>
class CopyDataClass
{
public:
	static void CopyEntity(DesType* ,SourceType*);
};

template <>
class CopyDataClass<CUserSessionField, CUserSession>
{
public:
	static void CopyEntity(CUserSessionField* pField, CUserSession* pData)
	{
// 		CopyUserSessionEntity(pField, pData);
	}	
};

template <>
class CopyDataClass<COrderField,COrder>
{
public:
	static void CopyEntity(COrderField* pField, COrder const* pData)
	{
		CopyZTPOrderEntity(pField, pData);
	}	
};

template <>
class CopyDataClass<CTradeField, CTrade>
{
public:
	static void CopyEntity(CTradeField* pField, CTrade* pData)
	{
		CopyTradeEntity(pField,pData);
	}	
};

template <>
class CopyDataClass<CInstrumentField, CInstrument>
{
public:
	static void CopyEntity(CInstrumentField* pField, CInstrument* pData)
	{
		CopyInstrumentInfoEntity(pField, pData);
	}	
};

template <>
class CopyDataClass<COperatorInfoField, COperatorInfo>
{
public:
	static void CopyEntity(COperatorInfoField* pField, COperatorInfo* pData)
	{
		CopyOperatorInfoEntity(pField, pData);
	}	
};

// template <>
// class CopyDataClass<CMarketDataField, CMarketData>
// {
// public:
// 	static void CopyEntity(CMarketDataField* pField, CMarketData* pData)
// 	{
// 		CopyMarketDataEntity(pField, pData);
// 	}	
// };
*/