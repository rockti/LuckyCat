#pragma once
#include "model/dealing/Dealing.h"
#include "service/Service.h"
#include "PresenterUtils.h"
#include <algorithm>

namespace ui {
	class OrderPresenter;

	class IOrderView : public ViewBase<OrderPresenter> {
	public:
		virtual void UpdateQuote(Quote const& quote) = 0;
		virtual void AddInstrument(Instrument const& inst) = 0;
	};
	class OrderPresenter : public IQuoteObserver, public IInstrumentObserver {
	public:
		OrderPresenter(IOrderView* view) :view_(view) {}
		void InsertOrder(OrderInsertRequest const& order);

		void CancelOrder(InvestorOrder const& order);

		virtual void OnQuote(Quote const& quote) override {
			view_->UpdateQuote(quote);
		}

		virtual void OnAdd(Instrument const& inst) override {
			view_->AddInstrument(inst);
		}

	private:
		IOrderView* view_;
	};
}
