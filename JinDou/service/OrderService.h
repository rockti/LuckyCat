#pragma once

#include "model/Constants.h"
#include "service/Service.h"

class OrderService : public ServiceBase<IOrderObserver,IOrderService> {
public:
	virtual void InitAdd(InvestorOrder const& order) override;
	void initLocalID(const ztp::InvestorOrder& order);
	virtual InvestorOrder const& AddOrUpdate(InvestorOrder const& order) override;
	virtual OrderResult GetOrder(type::OrderUniquID localID) const override;
	virtual OrderResult GetOrder(type::OrderLocalID localID) const override;
	virtual OrderResult GetOrder(type::OrderSysID const& sysNo) const override;
	virtual Orders GetAll() const override;
	virtual void OnTrade(InvestorTrade const& trade) override;
private:
	void updateSysIdIndex(InvestorOrder& order);
	void notifyAdd(InvestorOrder const& order);
	void notifyChange(InvestorOrder const& order);
	Deque<InvestorOrder> orders_;
	using OrderSysIdIndex = std::map<type::OrderSysID, InvestorOrder*, std::less<type::OrderSysID>,
		boost::fast_pool_allocator<std::pair<const type::OrderSysID, InvestorOrder*>>>;
	using OrderLocalIdIndex = std::map<type::OrderLocalID, InvestorOrder*, std::less<type::OrderLocalID>,
		boost::fast_pool_allocator<std::pair<const type::OrderLocalID, InvestorOrder*>>>;
	using OrderUniquIDIndex = std::map<type::OrderUniquID, InvestorOrder*, std::less<type::OrderUniquID>,
		boost::fast_pool_allocator<std::pair<const type::OrderUniquID, InvestorOrder*>>>;
	OrderSysIdIndex sysIdIndex_;
	OrderLocalIdIndex localIdIndex_;
	OrderUniquIDIndex uniqueIndex_;
	int index_ = 0;
};