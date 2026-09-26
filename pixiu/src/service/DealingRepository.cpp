#include "pch.h"

#include "service/DealingRepository.h"


DealingRepository& DealingRepository::Instance() {
	static DealingRepository s_repository;
	return s_repository;
}

Dealing& DealingRepository::Add(Dealing const& dealing) {
	auto [it, inserted] = m_dealings.insert(std::make_pair(dealing.GetLocalId(), dealing));
	assert(inserted);
	return it->second;
}

Dealing* DealingRepository::Get(LocalOrderID localID) {
	auto it = m_dealings.find(localID);
	if (it == m_dealings.end()) {
		return nullptr;
	}
	return &it->second;
}


void DealingRepository::Remove(LocalOrderID localID) {
	m_dealings.erase(localID);
}
