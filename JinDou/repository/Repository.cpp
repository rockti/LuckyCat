#include "Repository.h"
#include <sqlite_orm.h>
#include "service/CancellationLimiter.h"

using namespace sqlite_orm;

void RepositoryIniter::Init() {
	auto storage = make_storage("jindou.db",
		make_unique_index("idx_unique", &CancelCount::TradingDay, &CancelCount::InstID),
		make_table("t_CancelCount",
			make_column("TradeingDay", &CancelCount::TradingDay),
			make_column("InstID", &CancelCount::InstID),
			make_column("Max", &CancelCount::Max),
			make_column("Current", &CancelCount::Current)),
		make_table("t_LocalID",
			make_column("TradeingDay", &CancelCount::TradingDay),
			make_column("LocalID", &CancelCount::TradingDay)
			)
	);
}


void CancelCountRepository::LoadAll() {
}

void CancelCountRepository::Save(CancelCount const& cancelCount) {
}

void CancelCountRepository::Update(CancelCount const& cancelCount) {
}

