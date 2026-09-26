#pragma once
#include "IFlow.h"
#include "UserDataType.h"
#include "Scheduler.h"
#include "ZTPPackage.h"

class CScheduler
{
public:
	CScheduler(IFlow* pMemChangeFlow);
	virtual ~CScheduler();
	virtual void Dispatch(char const* pszObjectName, CFieldDescribe* pDescirbe, void const* pData, type::MemTableChangeType changeType);
	template<class Field>
	void Dispatch(char const* pszObjectName, Field const& object, type::MemTableChangeType changeType) {
		data::CMemTableChange field;
		field.MemTableName = pszObjectName;
		field.MemtableChangeType = changeType;

		m_pPubMessage->PreparePublish(TID_NtfMemtableChange);
		m_pPubMessage->AddField(field);
		m_pPubMessage->AddField(object);
		m_pPubMessage->MakePackage();
		m_pMemChangeFlow->Append(m_pPubMessage->GetAddress(), m_pPubMessage->GetLength());
	}
	virtual void InvestorAccountDepositNotify(CFieldDescribe *pDescirbe, const void *pData);
private:	
	IFlow* m_pMemChangeFlow;
	IFlow* m_pResultFlow;
	CZTPPackage* m_pPubMessage;
};