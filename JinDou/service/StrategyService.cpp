
#include "StrategyService.h"

void StrategyService::OnUpdate(int id, automate::AutoSnapshot const& snapshot) {
	for (auto& observer : observers_) {
		observer->OnUpdate(id, snapshot);
	}
}
