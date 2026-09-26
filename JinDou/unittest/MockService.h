#ifdef UNITTEST

#include <catch2/catch.hpp>

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>
#include <trompeloeil.hpp>
#include "service/Service.h"

namespace mock {
	class InitMockService : public IInitService {

	public:
		MAKE_MOCK1(Register, void(IInitProgressObserver* observer), override);
		MAKE_MOCK1(Unregister, void(IInitProgressObserver* observer), override);
		MAKE_MOCK1(AddPlatform, void(TradePlatform platform), override);
		MAKE_MOCK1(DeletePlatform,void(TradePlatform platform), override);
		MAKE_MOCK1(ResetPlatform,void(TradePlatform platform), override);
		MAKE_MOCK4(OnStatusUpdate, void(TradePlatform platform, InitProcess process, InitStatus status, Error error), override);
		MAKE_MOCK1(GetPercent, int(TradePlatform platform), override);
		MAKE_MOCK1(GetLastMessage, std::string(TradePlatform platform), override);
		MAKE_MOCK1(GetLastError,Error(TradePlatform platform), override);
		MAKE_CONST_MOCK1(Has, bool(TradePlatform platform), override);
	};

	class QuoteMockService : public IQuoteService {

	public:
		MAKE_MOCK1(Register,void(IQuoteObserver* observer),override);
		MAKE_MOCK1(Unregister,void(IQuoteObserver* observer),override);

		MAKE_MOCK1(Update, Quote const& (Quote const& quote),override);
		MAKE_MOCK4(UpdatePrice, Quote const& (type::InstID instID, SimpleQuoteArray const& buyArray, SimpleQuoteArray const& sellArray, type::Money lastPrice),override);
		MAKE_CONST_MOCK1(Get, QuoteResult(type::InstID inst),override);
	};
	
	class PositionMockService : public IPositionService {

	public:
		MAKE_MOCK1(Register, void(IPositionObserver* observer), override);
		MAKE_MOCK1(Unregister, void(IPositionObserver* observer), override);

		MAKE_MOCK1(InitAdd, InvestorPosition const& (InvestorPosition const& position), override);
		MAKE_CONST_MOCK2(Get, PositionResult(type::InstID inst, type::Direction direction), override) ;
		MAKE_CONST_MOCK2(Has, bool(type::InstID inst, type::Direction direction), override);
		MAKE_MOCK1(Get, InstrumentPositionResult(type::InstID inst), override);
		MAKE_CONST_MOCK0(All, Positions(), override);
		MAKE_MOCK0(ClearAll, void(), override);
		MAKE_MOCK1(ClearPlatform, void(TradePlatform platform) , override);
		MAKE_MOCK1(OnQuote, void(Quote const& quote) , override);
		MAKE_MOCK1(OnTrade, void(InvestorTrade const& trade) , override);
		MAKE_MOCK1(FreezePosition, void(InvestorOrder const& order) , override);
		MAKE_MOCK1(UnfreezePosition, void(InvestorOrder const& order) , override);
	};

	class TradeSystemMockService : public ITradeSystemService {

	public:
		MAKE_MOCK1(Register, void(IExchangeStatusObserver* observer), override);
		MAKE_MOCK1(Unregister, void(IExchangeStatusObserver* observer), override);
		MAKE_MOCK1(Register, void(ITradeSystemObserver* observer), override);
		MAKE_MOCK1(Unregister, void(ITradeSystemObserver* observer), override);


		MAKE_MOCK3(Login, void(TradePlatform platform, type::InvestorID const& investor, type::Password const& password) , override);
		MAKE_MOCK5(Login, void(TradePlatform platform, type::InvestorID const& tradeInvestor, type::Password const& tradePassword,	type::InvestorID const& quoteInvestor, type::Password const& quotePassword), override);
		MAKE_MOCK2(ReLogin, void(TradePlatform platform, SubSystem subSystem), override);
		MAKE_CONST_MOCK2(IsConnected, bool(TradePlatform platform, SubSystem subSystem) , override);
		MAKE_CONST_MOCK2(IsLogined, bool(TradePlatform platform, SubSystem subSystem) , override);
		MAKE_MOCK3(ChangePassword, void(TradePlatform platform, type::Password const& oldPassword, type::Password const& newPassword), override);

		MAKE_MOCK1(Insert, void(OrderInsertRequest const& order) , override);
		MAKE_MOCK1(Cancel,void(OrderCancelRequest const& action) , override);
		MAKE_MOCK2(SubscribeQuote,void(type::Exchange exchange, type::InstrumentCode const& code) , override);
		MAKE_MOCK0(RunQuery, void() , override);

		MAKE_MOCK1(QueryInstruments, void(TradePlatform platform) , override);
		MAKE_MOCK1(QueryAccount, void(TradePlatform platform) , override);
		MAKE_MOCK1(QueryPositions, void(TradePlatform platform) , override);
		MAKE_MOCK1(QueryOrders, void(TradePlatform platform) , override);
		MAKE_MOCK1(QueryTrades, void(TradePlatform platform) , override);
		MAKE_MOCK1(QueryFees, void(TradePlatform platform) , override);
		MAKE_MOCK1(QueryMargins, void(TradePlatform platform) , override);
		MAKE_MOCK2(QueryFee, void(TradePlatform platform, type::InstrumentCode const& code) , override);
		MAKE_MOCK2(QueryMargin, void(TradePlatform platform, type::InstrumentCode const& code) , override);
		MAKE_MOCK2(OnConnected, void(TradePlatform platform, SubSystem subSystem) , override);
		MAKE_MOCK2(OnDisconnected, void(TradePlatform platform, SubSystem subSystem) , override);
		MAKE_MOCK2(OnLogin, void(TradePlatform platform, SubSystem subSystem) , override);
		MAKE_MOCK2(OnLogout, void(TradePlatform platform, SubSystem subSystem) , override);
		MAKE_MOCK2(OnStatus, void(type::Exchange, ExchangeStatus status) , override);
		MAKE_MOCK3(OnLoginFailed, void(TradePlatform platform, SubSystem subSystem, Error error) , override);
		MAKE_MOCK3(OnPasswordChange, void(TradePlatform platform, SubSystem subSystem, Error error) , override);

		MAKE_MOCK1(OnOrderError, void(OrderErrorNotification const& en) , override);
		MAKE_MOCK1(OnError, void(ErrorNotification const& en) , override);

		MAKE_CONST_MOCK0(GetTradingDay, type::TradingDay const& (), override);
		MAKE_MOCK1(SetTradingDay, void(type::TradingDay const& tradingDay), override);
	};
}
#endif
