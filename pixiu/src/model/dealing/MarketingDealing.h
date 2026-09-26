#pragma once
#include "model/Constants.h"
#include "model/dealing/Dealing.h"


class Business;

typedef uint32_t DealingID;

static int s_marketing_dealing_id = 0;

class MarketingDealing {
public:
	MarketingDealing(Dealing& buyDealing, Dealing& sellDealing)
		:m_id(s_marketing_dealing_id++),m_buyDealing(&buyDealing),
		m_sellDealing(&sellDealing)
	{
	}

	MarketingDealing()
		:m_id(-1), m_buyDealing(nullptr),	m_sellDealing(nullptr) {
	}

	MarketingDealing& operator=(MarketingDealing const& other) {
		m_id = other.m_id;
		m_buyDealing = other.m_buyDealing;
		m_sellDealing = other.m_sellDealing;
		return *this;
	}
	inline void SetBusiness(Business* business) { m_buyDealing->SetBusiness(business); m_sellDealing->SetBusiness(business); }
	inline Business* GetBusiness() const {	return m_buyDealing->GetBusiness(); }
	inline int GetID() const { return m_id; }
	inline LocalOrderID GetLocalId() const { return m_buyDealing->GetLocalId(); }
	Dealing& GetBuyDealing() const { return *m_buyDealing; }
	Dealing& GetSellDealing() const{ return *m_sellDealing; }
	inline bool IsAllCancelled() const { return m_buyDealing->IsCancelled() && m_sellDealing->IsCancelled(); }
	inline bool IsAllOrdered() const { return m_buyDealing->IsOrdered() && m_sellDealing->IsOrdered(); }
	inline bool IsAllFinished() const { return m_buyDealing->IsFinished() && m_sellDealing->IsFinished(); }
	inline Exchange GetExchange() const { return Exchange::HKEX; } // only hk support now

private:
	Dealing* m_buyDealing;
	Dealing* m_sellDealing;
	int m_id;
};

