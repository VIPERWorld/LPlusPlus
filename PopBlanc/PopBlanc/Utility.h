#pragma once
#include <vector>
class Vec3;
class IUnit;

class Utility
{
public:
	Utility();
	~Utility();

	static bool IsValidUnit(IUnit* unit);
	static bool InRange(IUnit* unit, float range);
	static int CountBuffs(std::vector<IUnit*> units, const char* buff);
	static int CountMinionsInRange(const Vec3 position, float range);
	static std::vector<IUnit*> GetEnemiesInRange(IUnit* unit, float range);
	static std::vector<IUnit*> GetAlliesInRange(IUnit* unit, float range);
	static int CountEnemiesInRange(IUnit* unit, float range);
	static int CountAlliesInRange(IUnit* unit, float range);
	static bool IsValidTarget(IUnit* unit, float range = 50000);
	static bool IsPassiveActive(IUnit* unit, float delay = 0);
	static int CountEnemiesWithPassive(IUnit* unit, float range);
private:
	static char* PassiveBuff;
	static std::vector<IUnit*> GetInRange(IUnit* unit, float range, std::vector<IUnit*> units);
	static int CountInRange(IUnit* unit, float range, std::vector<IUnit*> units);
};

