#include "runner/UIRunner.h"
#include "notify/Notify.h"

void UIRunnerHandler::Handle(Data const& data) {
	switch (data.Type) {
	case DataType::UIMessage: {

	}
	case DataType::Exchange: {
		TradePlatformNotification notif = *(TradePlatformNotification*)data.Payload;
		break;
	}
	case DataType::Position: {
		break;
	}

	case DataType::Account: {
		break;
	}
	default:
		throw ILLEGAL_MESSAGE_TYPE;
	}
}
