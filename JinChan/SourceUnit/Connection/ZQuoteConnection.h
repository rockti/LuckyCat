#pragma once
#include "ZQuoteHandler.h"
#include "ZQuote/ZQQuoteAPI.h"


class ZQuoteConnection {
public:
	ZQuoteConnection(IFlow* flow);
	~ZQuoteConnection();
	void Login();
	void SubMarket();
private:
	IZQQuoteAPI* quoteAPI_ = nullptr;
	ZQuoteQuoteHandler quoteHandler_;
};