#include "pch.h"

#include "service/FundService.h"
#include "service/QuoteService.h"
#include "model/dealing/Dealing.h"
// 
// FundsService& FundsService::Instance() {
// 	static FundsService s_service;
// 	return s_service;
// }
// 
// void FundsService::Freeze(Exchange exchange, LocalOrderID localID, money freeze) {
// 	Funds& funds = GetFunds(exchange);
// 	funds.UnFreeze(localID);
// 	funds.Freeze(localID, freeze);
// }
// 
// money FundsService::FreezeDealingFunds(Dealing const& dealing)
// {
// 	QuoteService& quoteService = WritableQuoteService();
// 	Funds& funds = GetFunds(dealing.GetExchange());
// 	funds.UnFreeze(dealing.GetLocalId());
// 	if (dealing.GetRemainQuantity() > 0)
// 	{
// 		Quote const& quote = quoteService.GetQuote(dealing.GetInstID());
// 		money freezeAmount = quote.CalculateTotalFreeze(dealing.GetOrderPrice(), dealing.GetRemainQuantity(), dealing.GetOperation());
// 		funds.Freeze(dealing.GetLocalId(), freezeAmount);
// 		return freezeAmount;
// 	}
// 	return 0.00;
// }
// 
// void FundsService::OnDealing(Dealing const& dealing)
// {
// 	FreezeDealingFunds(dealing);
// }
// void FundsService::OnTrade(Trade const& trade)
// {
// 	FreezeDealingFunds(trade.GetDealing());
// 	if (trade.GetDealing().IsOpenOrder()) {
// 		DealWithOpenTrade(trade);
// 	} else {
// 		DealWithOffsetTrade(trade);
// 	}
// }
// 
// bool FundsService::Enough(Exchange exchange, money m) {
// 	return GetFunds(exchange).HaveEnoughCapital(m);
// }
// 
// Operation FundsService::SubdivideOffsetOperation(Trade const& trade)
// {
// 	return ((trade.GetDealing().GetOperation() == Operation::OFFSET) ? Operation::OFFSET : Operation::OFFSET_TODAY);
// }
// 
// void FundsService::DealWithOpenTrade(Trade const& trade)
// {
// 	QuoteService& quoteService = WritableQuoteService();
// 	Funds& funds = GetFunds(trade.GetDealing().GetExchange());
// 	Quote const& quote = quoteService.GetQuote(trade.GetInstID());
// 	money openMargin = quote.CalculateMargin(trade.GetDealPrice(), trade.GetDealQuantity(), Operation::OPEN);
// 	money openFee = quote.CalculateFee(trade.GetDealPrice(), trade.GetDealQuantity(), Operation::OPEN);
// 	funds.OpenHold(openMargin, openFee);
// }
// void FundsService::DealWithOffsetTrade(Trade const& trade)
// {
// 	QuoteService& quoteService = WritableQuoteService();
// 	Funds& funds = GetFunds(trade.GetDealing().GetExchange());
// 
// 	Dealing const& dealing = trade.GetDealing();
// 	Quote const& quote = quoteService.GetQuote(trade.GetInstID());
// 	
// 	money returnMargin = quote.CalculateMargin(trade.GetDealPrice(), trade.GetDealQuantity(), Operation::OPEN);
// 	money offsetFee = quote.CalculateFee(trade.GetDealPrice(), trade.GetDealQuantity(), dealing.GetOperation());
// 	funds.OffsetRelease(returnMargin, offsetFee);
// }
// 
