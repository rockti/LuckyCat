#pragma once

#include "model/Constants.h"
#include "service/Service.h"

struct TradePlatformProgress {
	struct StatusWithError {
		InitStatus Status = InitStatus::New;
		Error Error = NoError;
	};
	int Percent = 0;
	std::string LastMessage;
	Error LastError = NoError;
	std::map<InitProcess, StatusWithError> Processes;

	void UpdateProcess(InitProcess process, InitStatus status, Error error) {
		if (Processes[process].Status != InitStatus::Done) {
			Processes[process].Status = status;
			Processes[process].Error = error;
			Percent = recalcPersent();
		}
	}
	void Reset() {
//		Processes;
		Percent = 0;
		LastMessage = "";
		LastError = NoError;
	}
	const int TotalProcess = 12;
private:
	int recalcPersent() const {
		double percent = 0;
		for (auto const& kv : Processes) {
			if (kv.second.Status == InitStatus::Done) {
				percent++;
			}
		}
		return int((percent / TotalProcess) * 100);
	}
};

class InitService : public ServiceBase<IInitProgressObserver,IInitService> {
public:
	InitService();

	virtual void AddPlatform(TradePlatform platform) override;
	virtual void DeletePlatform(TradePlatform platform) override;
	virtual void OnStatusUpdate(TradePlatform platform, InitProcess process, InitStatus status, Error error) override;
	virtual int GetPercent(TradePlatform platform) override;
	virtual std::string GetLastMessage(TradePlatform platform) override;

	virtual bool Has(TradePlatform platform) const override;


	virtual void ResetPlatform(TradePlatform platform) override;


	virtual Error GetLastError(TradePlatform platform) override;

private:
	std::string getMessage(InitProcess process, InitStatus status, Error error);
	std::string getStatusString(InitStatus status, Error error);
	using ProgressMap = std::map<TradePlatform, TradePlatformProgress>;
	ProgressMap progresses_;
	std::array<bool, 10> isInitDone_ = { false };
//	bool isInitDone_ = false;
	void handleStatusChange(TradePlatform platform, InitProcess process, InitStatus status);
};
