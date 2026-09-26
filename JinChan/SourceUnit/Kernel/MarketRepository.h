#pragma once
#include "EventHandler.h"
#include "FtdDataStruct.h"
#include "MarketFlowMgr.h"
//#include "HashMap.h"
#include "FTDPackage.h"
#include "MarketEngine.h"

typedef std::map<CInstrumentIDType, CFTDDepthMarketDataField*> MarketDataMap;	//行情Map<合约, 行情数据>
typedef std::map<int, MarketDataMap*> MarketTopicMap;							//主题Map<主题, 行情Map>
typedef std::map< CInstrumentIDType, DWORD> InstrumentIndexMap;
/*
//管理行情快照及合约并对其编号
class CMarketRepository  {//: public CEventHandler
public:
	CMarketRepository(CReactorThread* pReactor);
	~CMarketRepository();

	virtual void OnTimer(int nIDEvent);

	//获取合约对应的id号
	DWORD GetInstrumentIndex(CInstrumentIDType instrmentID);

	//插入最新行情记录,生成此合约的id号
	DWORD InsertMaraketData(int nTopicID, CFTDDepthMarketDataField* pMarketData);

	//返回给行情查询的数据接口
	CFTDDepthMarketDataField* GetMaraketData(CInstrumentIDType InstrumentID);	
	MarketTopicMap const& GetMarketTopicMap() const { return m_marketTopicMap; }
	InstrumentIndexMap* GetInstrumentIndexMap()	{ return &m_instrumentIndexMap; }

private:
	int m_udpFlag;		//是否内部广播
	CChannel* m_udpChannel;	
	CFTDPackage* m_pFTDPackage;
	DWORD m_instrumentCount;

	MarketTopicMap m_marketTopicMap;
	InstrumentIndexMap m_instrumentIndexMap;
	CFTDDepthMarketDataField m_ftdMarketDataField;
	CMutex m_lock;
};

*/