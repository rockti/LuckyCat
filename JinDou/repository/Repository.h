#pragma once
struct CancelCount;

class RepositoryIniter {
public:
	static void Init();
};

class CancelCountRepository {
public:
	void LoadAll();
	void Save(CancelCount const& cancelCount);
	void Update(CancelCount const& cancelCount);
private:

};