#include "stdafx.h"
#include "RateAlgorithms.h"

CAlgorithms::CAlgorithms()
{
}
CAlgorithms::~CAlgorithms()
{
}

double CAlgorithms::CalculateFee(type::Volume Volume, type::Price orderPrice, type::OffsetFlag offsetFlag, 
	type::VolumeMultiple multiple, data::CInvestorFee const* pInvestorFee) {
	if(type::OffsetFlag::Open == offsetFlag) {
		return Volume * orderPrice * multiple * pInvestorFee->OpenFeeRate + Volume * pInvestorFee->OpenFeeAmt;
	} else {
//		return Volume * orderPrice * multiple * pInvestorFee->OffsetFeeRate + Volume * pInvestorFee->OffsetFeeAmt;
		if (type::OffsetFlag::CloseToday == offsetFlag) {
			return Volume*orderPrice*multiple*pInvestorFee->OffsetTodayFeeRate + Volume*pInvestorFee->OffsetTodayFeeAmt;
		} else {
			return Volume*orderPrice*multiple*pInvestorFee->OffsetFeeRate + Volume*pInvestorFee->OffsetFeeAmt;
		}
	}
}
double CAlgorithms::CalculateMargin(type::Volume Volume, type::Price openPrice, type::Direction direction, 
	type::VolumeMultiple multiple, data::CInvestorMargin const* pInvestorMargin) {
	double marginAmt = type::IsBuy(direction) ? pInvestorMargin->LongMarginAmt : pInvestorMargin->ShortMarginAmt;
	double marginRate = type::IsBuy(direction) ? pInvestorMargin->LongMarginRate : pInvestorMargin->ShortMarginRate;
	return Volume * openPrice * multiple * marginRate + Volume * marginAmt;
}
