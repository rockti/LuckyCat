#pragma once
#include "ZTPType.h"

using namespace ztp;
class CAlgorithms
{
public:
	CAlgorithms();
	virtual ~CAlgorithms();

	double CalculateFee(type::Volume Volume, type::Price orderPrice, type::OffsetFlag offsetFlag, 
		type::VolumeMultiple multiple, data::CInvestorFee const* pInvestorFee);
	double CalculateMargin(type::Volume Volume, type::Price openPrice, type::Direction direction, 
		type::VolumeMultiple multiple, data::CInvestorMargin const* pInvestorMargin);
};
