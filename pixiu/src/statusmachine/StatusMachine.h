#pragma once

#include "model/Constants.h"
#include "model/dealing/Dealing.h"
#include <array>
#include "metrics/MovingAverage.h"

// 	enum class Status {
// 		Start = 0,
// 		OpenSent,
// 		Opened,
// 		ProfitSent,
// 		LostSent,
// 		End,
// 	};
// 
// 	class SignalBusiness;
// 	struct BaseStatus {
// 		BaseStatus(SignalBusiness* machine, Status status) :m_business(machine), m_index(status) {}
// 		virtual BaseStatus* OnQuote(QuoteTicks const& ticks, Crossover cross) { return this; }
// 
// 		virtual BaseStatus* OnOrder() { return this; }
// 		virtual BaseStatus* OnTrade(QtyT qty, TickT price) { return this; }
// 
// 		virtual BaseStatus* OnCancel() { return this; }
// 
// 		virtual BaseStatus* OnCancelCommand() { return this; }
// 
// 		Status m_index;
// 		SignalBusiness* m_business;
// 	};
// 
// 	struct StartStatus : public BaseStatus {
// 		StartStatus(SignalBusiness* machine) :BaseStatus(machine, Status::Start) {}
// 		virtual BaseStatus* OnQuote(QuoteTicks const& ticks, Crossover cross) override;
// 	};
// 
// 	struct OpenSentStatus : public BaseStatus {
// 		OpenSentStatus(SignalBusiness* machine) :BaseStatus(machine, Status::OpenSent) {}
// 
// 		virtual BaseStatus* OnOrder() override;
// 		virtual BaseStatus* OnTrade(QtyT qty, TickT price) override;
// 		virtual BaseStatus* OnCancel() override;
// 
// 	};
// 
// 	struct OpenedStatus : public BaseStatus {
// 		OpenedStatus(SignalBusiness* machine) :BaseStatus(machine, Status::Opened) {}
// 
// 		virtual BaseStatus* OnQuote(QuoteTicks const& ticks, Crossover cross) override;
// 	};
// 
// 	struct ProfitSentStatus : public BaseStatus {
// 		ProfitSentStatus(SignalBusiness* machine) :BaseStatus(machine, Status::ProfitSent) {}
// 
// 		virtual BaseStatus* OnOrder() override;
// 		virtual BaseStatus* OnTrade(QtyT qty, TickT price) override;
// 		virtual BaseStatus* OnCancel() override;
// 
// 	};
// 
// 	struct LostSentStatus : public BaseStatus {
// 		LostSentStatus(SignalBusiness* machine) :BaseStatus(machine, Status::LostSent) {}
// 
// 		virtual BaseStatus* OnOrder() override;
// 		virtual BaseStatus* OnTrade(QtyT qty, TickT price) override;
// 		virtual BaseStatus* OnCancel() override;
// 
// 	};
// 
// 	struct EndStatus : public BaseStatus {
// 		EndStatus(SignalBusiness* machine) :BaseStatus(machine, Status::End) {}
// 	};

