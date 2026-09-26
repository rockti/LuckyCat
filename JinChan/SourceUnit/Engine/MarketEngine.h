#pragma once
#include "MarketScheduler.h"
#include "ZTPPackage.h"
#include "DBWorkThread.h"

class CMarketEngine
{
public:
	CMarketEngine(CMarketScheduler* pMarketSchulder);
	void HandleMessage(CZTPPackage* pPackage);
	void HandleMemberDBMsg(CZTPPackage* pPackage);
	void ProcField(WORD fid, CFieldTypeIterator& iter, TMDBActionType actType);

private:
	CMarketScheduler* m_pMarketSchulder;
	int m_count;
};