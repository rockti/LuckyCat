#pragma once
#include "ZTPPackage.h"
#include "FunctionCopy.h"


template <typename T, typename BaseTrigger>
class CNewUsTrigger : public BaseTrigger {
public:
	///构造方法
	CNewUsTrigger(CScheduler* pMdbSch, char const* pszObjectName){
		m_pScheduler = pMdbSch;
		m_pszObjectName = pszObjectName;
	}

	virtual ~CNewUsTrigger(void) {}

	virtual void OnAdd(T const& object) {
		m_pScheduler->Dispatch(m_pszObjectName, object, type::MemTableChangeType::Insert);
	}
	virtual void OnUpdate(T const& object) {
		m_pScheduler->Dispatch(m_pszObjectName, object, type::MemTableChangeType::Update);
	}
	virtual void OnDelete(T const& object) {
		m_pScheduler->Dispatch(m_pszObjectName, object, type::MemTableChangeType::Delete);
	}

protected:

protected:
	CScheduler* m_pScheduler;
	char const* m_pszObjectName;
};


