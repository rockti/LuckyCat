
#include "service/ExchangeService.h"

TradePlatform ExchangeService::toPlatform(type::Exchange exchange) {
	switch (exchange) {
	case ztp::type::Exchange::SHFE:
		return TradePlatform::CTP;
	case ztp::type::Exchange::HKEX:
		return TradePlatform::HKEX_ESUNNY;
	case ztp::type::Exchange::CME:
		return TradePlatform::HKEX_ESUNNY;
	case ztp::type::Exchange::CFFEX:
		return TradePlatform::CTP;
	case ztp::type::Exchange::SGE:
		return TradePlatform::ZTP;
	case ztp::type::Exchange::DCE:
		return TradePlatform::CTP;
	case ztp::type::Exchange::CZCE:
		return TradePlatform::CTP;
	default:
		return TradePlatform::CTP;
		break;
	}
}
