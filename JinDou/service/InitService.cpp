
#include "service/InitService.h"
#include "utils/Log.h"
InitService::InitService() {
}

void InitService::AddPlatform(TradePlatform platform) {
	auto iter = progresses_.find(platform);
	if (iter != progresses_.end()) {
		iter->second.Reset();
	} else {
		progresses_.emplace(platform, TradePlatformProgress{});
	}
}

void InitService::DeletePlatform(TradePlatform platform) {
	progresses_.erase(platform);
}


void InitService::OnStatusUpdate(TradePlatform platform, InitProcess process, InitStatus status, Error error) {
	Log::Error(fmt::format("xxxx P[{}] P[{}] S[{}] E[{}]", platform, process, status, error));
	auto iter = progresses_.find(platform);
	if (iter != progresses_.end()) {
		iter->second.UpdateProcess(process, status, error);

		handleStatusChange(platform, process, status);

		auto msg = getMessage(process, status, error);
		iter->second.LastMessage = msg;
		if (!error) {
			iter->second.LastError = error;
		}
		for (auto& observer : observers_) {
			observer->OnProgress(platform, iter->second.Percent, msg);
		}
	}
}

int InitService::GetPercent(TradePlatform platform) {
	auto iter = progresses_.find(platform);
	if (iter != progresses_.end()) {
		return iter->second.Percent; 
	}
	return 100;
}

std::string InitService::GetLastMessage(TradePlatform platform) {
	auto iter = progresses_.find(platform);
	if (iter != progresses_.end()) {
		return iter->second.LastMessage;
	}
	return "";
}

bool InitService::Has(TradePlatform platform) const {
	auto iter = progresses_.find(platform);
	return iter != progresses_.end();
}

void InitService::ResetPlatform(TradePlatform platform) {
	auto iter = progresses_.find(platform);
	if (iter != progresses_.end()) {
		iter->second.Reset();
	}
}

Error InitService::GetLastError(TradePlatform platform) {
	auto iter = progresses_.find(platform);
	if (iter != progresses_.end()) {
		return iter->second.LastError;
	}
	return NoError;
}

std::string InitService::getMessage(InitProcess process,InitStatus status, Error error) {
	switch (process)
	{
	case InitProcess::QuoteConnect:
		return fmt::format("连接行情-{}", getStatusString(status, error));
	case InitProcess::TradeConnect:
		return fmt::format("连接交易-{}", getStatusString(status, error));
	case InitProcess::QuoteLogin:
		return fmt::format("登录行情-{}", getStatusString(status, error));
	case InitProcess::TradeLogin:
		return fmt::format("登录交易-{}", getStatusString(status, error));
	case InitProcess::QueryInstrument:
		return fmt::format("查询合约-{}", getStatusString(status, error));
	case InitProcess::QueryFee:
		return fmt::format("查询费率-{}", getStatusString(status, error));
	case InitProcess::QueryMargin:
		return fmt::format("查询保证金率-{}", getStatusString(status, error));
	case InitProcess::SubscribeQuote:
		return fmt::format("订阅行情-{}", getStatusString(status, error));
	case InitProcess::QueryAccount:
		return fmt::format("查询资金-{}", getStatusString(status, error));
	case InitProcess::QueryPosition:
		return fmt::format("查询持仓-{}", getStatusString(status, error));
	case InitProcess::QueryOrder:
		return fmt::format("查询订单-{}", getStatusString(status, error));
	case InitProcess::QueryTrade:
		return fmt::format("查询成交-{}", getStatusString(status, error));
	default:
		return "";
	}
}

std::string InitService::getStatusString(InitStatus status, Error error) {
	switch (status)
	{
	case InitStatus::New:
		return "未开始";
	case InitStatus::Processing:
		return "正在处理";
	case InitStatus::Done:
		return "成功";
		break;
	case InitStatus::Fail:
		return fmt::format("失败，失败原因{}", error.Msg);
	default:
		return "未开始";
	}

}

void InitService::handleStatusChange(TradePlatform platform, InitProcess process, InitStatus status) {
	auto* tradeService = ServiceSinglton::Services().TradeSystem;
	if (isInitDone_[int(platform)]) return;
	switch (process) {
	case InitProcess::QuoteConnect:
	case InitProcess::TradeConnect:
	case InitProcess::QuoteLogin:
		break;
	case InitProcess::TradeLogin:
		if (status == InitStatus::Done) {
			tradeService->QueryAccount(platform);			// login success -> query account
		}
		break;
	case InitProcess::QueryAccount:
		if (status == InitStatus::Done) {
			if (GetPercent(platform) != 100) {
				tradeService->QueryInstruments(platform);	// account success -> query instrument
			}
		}
		break;
	case InitProcess::QueryInstrument:
		if (status == InitStatus::Done) {
			tradeService->QueryFees(platform);				// instrument success -> query fees
		}
		break;
	case InitProcess::QueryFee:
		if (status == InitStatus::Done) {
			tradeService->QueryMargins(platform);			// fees success -> query margins
		}
		break;
	case InitProcess::QueryMargin:
		if (status == InitStatus::Done) {
			tradeService->QueryPositions(platform);			// margins success -> query position
		}
		break;
	case InitProcess::QueryPosition:
		if (status == InitStatus::Done) {
			tradeService->QueryOrders(platform);			// position success -> query orders
		}
		break;
	case InitProcess::QueryOrder:
		if (status == InitStatus::Done) {
			tradeService->QueryTrades(platform);			// orders success -> query trades
		}
		break;
	case InitProcess::QueryTrade:
		if (status == InitStatus::Done) {
			isInitDone_[int(platform)] = true;
		}
		break;
	case InitProcess::SubscribeQuote:						// subscribe instrument is on dispatcher
		break;
	default:
		break;
	}
}
