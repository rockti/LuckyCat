#include "CancellationLimiter.h"

bool CancellationLimiter::IsCountCancel(InvestorOrder const& order) {
    if (order.IsCancelled() && order.OrderPriceType == type::OrderPriceType::LimitPrice) {
        return true;
    }
    return false;
}
