#include <map>
#include "PluginSDK.h"
#include "Utility.h"
#include <string>
PluginSetup("PopBlanc - Trees");

IMenu* MainMenu;
IMenu* QMenu;
IMenu* WMenu;
IMenu* EMenu;
IMenu* RMenu;
IMenu* FleeMenu;
IMenu* Misc;
IMenu* Drawings;

IMenuOption* ComboQ;
IMenuOption* HarassQ;
IMenuOption* HarassQBuff;
IMenuOption* LastHitQ;
IMenuOption* LaneClearQ;
IMenuOption* FarmQMana;

IMenuOption* ComboW;
IMenuOption* HarassW;
IMenuOption* FarmW;

IMenuOption* ComboE;
IMenuOption* EFirst;

IMenuOption* ComboR;
IMenuOption* ComboREnemies;
IMenuOption* DrawReady;

IMenuOption* FleeKey;
IMenuOption* FleeW;
IMenuOption* FleeRW;

IMenuOption* DrawQ;
IMenuOption* DrawW;
IMenuOption* DrawE;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;

IUnit* Player;

std::map<ISpell2*, IMenuOption*> SpellMenuMap;

const char* QBuff = "LeblancPMark";
const char* EBuff = "LeblancE";

static bool IsKeyDown(IMenuOption* menuOption)
{
	return (GetAsyncKeyState(menuOption->GetInteger()) & 0x8000) != 0;
}

bool IsPassiveActive(IUnit* unit, float delay = 0)
{
	auto data = unit->GetBuffDataByName(QBuff);

	if (GBuffData->IsValid(data) && GBuffData->IsActive(data))
	{
		auto duration = (GGame->Time() - GBuffData->GetStartTime(data)) * 1000;
		// can improve this to take into account spell delay + ping/2
		return duration >= 1400 && duration < 3800;
	}

	return false;
}

IUnit* GetTarget()
{
	auto range = E->IsReady() ? E->Range() : W->Range();

	for (auto enemy : GEntityList->GetAllHeros(false, true))
	{
		if (!Utility::IsValidTarget(enemy, range))
		{
			continue;
		}

		if (IsPassiveActive(enemy) || enemy->HasBuff(EBuff))
		{
			return enemy;
		}
	}

	return GTargetSelector->FindTarget(QuickestKill, SpellDamage, range);
}


bool bCastSpell(ISpell2* spell, IUnit* target)
{
	auto opt = SpellMenuMap[spell];
	return opt != nullptr && opt->Enabled() && spell->IsReady() && Utility::IsValidTarget(target, spell->Range());
}

bool IsFirstW()
{
	return strcmp(Player->GetSpellBook()->GetName(kSlotW), "LeblancW") == 0;
}

bool IsFirstR()
{
	return Player->GetSpellBook()->GetToggleState(kSlotR) == 1;
}

bool IsRActive()
{
	return Player->GetSpellBook()->GetToggleState(kSlotR) == 2;
}

bool CastSecondW()
{
	// Second W Enabled
	return GOrbwalking->GetOrbwalkingMode() == kModeMixed && W->IsReady() && !Q->IsReady() && !IsFirstW() && W->CastOnPlayer();
}

bool CastRSpell(IUnit* target)
{
	if (!IsRActive())
	{
		return false;
	}

	// must be reworked
	if (Utility::CountBuffs(Utility::GetEnemiesInRange(target, 500), QBuff) > 0)
	{
		if (bCastSpell(W, target) && IsFirstW() && W->CastOnTargetAoE(target, 3, kHitChanceMedium))
		{
			return true;
		}

		if (!bCastSpell(Q, target))
		{
			return false;
		}

		auto hasBuff = IsPassiveActive(target);
		if ((target->GetHealth() < GDamage->GetSpellDamage(Player, target, kSlotQ, hasBuff ? 1 : 0) || hasBuff) && Q->CastOnTarget(target))
		{
			return true;
		}
	}

	return bCastSpell(E, target) && (target->ServerPosition() - Player->ServerPosition()).Length() > 40 && E->CastOnUnit(target);
}

void PrintBuff(IUnit* unit)
{
	std::vector<void*> vecBuffs;
	unit->GetAllBuffsData(vecBuffs);

	for (auto i : vecBuffs)
	{
		GBuffData->GetBuffName(i);
		GUtility->LogConsole(GBuffData->GetBuffName(i));
	}
}

void Combo(IUnit* targ = nullptr, bool force = false)
{
	try
	{
		auto target = GetTarget();

		if (!Utility::IsValidTarget(target))
		{
			return;
		}

		if (CastRSpell(target))
		{
			return;
		}

		if (bCastSpell(W, target) && IsFirstW() && W->CastOnTarget(target, kHitChanceMedium))
		{
			return;
		}

		if (bCastSpell(E, target) && (target->ServerPosition() - Player->ServerPosition()).Length() > 40 && E->CastOnUnit(target))
		{
			return;
		}

		if (bCastSpell(Q, target) && Q->CastOnUnit(target))
		{
			return;
		}

		if (ComboR->Enabled() && R->IsReady() && IsFirstR() && R->CastOnPlayer())
		{
			return;
		}
	}
	catch (...)
	{
		GGame->PrintChat("FUCK");
	}
}

void Harass()
{
	if (!HarassQ->Enabled() && !HarassW->Enabled())
	{
		return;
	}

	auto target = GetTarget();

	if (!Utility::IsValidTarget(target, W->Range()))
	{
		return;
	}

	if (W->IsReady() && IsFirstW() && W->CastOnTarget(target, kHitChanceMedium))
	{
		return;
	}

	// this should work
	if (Q->IsReady())
	{
		if (!HarassQBuff->Enabled() && !IsPassiveActive(target))
		{
			return;
		}

		if (Utility::IsValidTarget(target, Q->Range()) && Q->CastOnUnit(target))
		{
			return;
		}

		for (auto obj : GEntityList->GetAllUnits())
		{
			if (Utility::IsValidUnit(obj) && obj->IsValidTarget(Player, Q->Range()) && IsPassiveActive(obj) && (obj->ServerPosition() - target->ServerPosition()).Length() < 500 && Q->CastOnUnit(obj))
			{
				return;
			}
		}
	}
}

void LastHit()
{
	if (!LastHitQ->Enabled() || !Q->IsReady())
	{
		return;
	}

	for (auto minion : GEntityList->GetAllMinions(false, true, false))
	{
		auto dmg = GDamage->GetSpellDamage(Player, minion, kSlotQ, 0);
		if (Utility::IsValidTarget(minion, Q->Range()) || minion->GetHealth() > dmg)
		{
			continue;
		}

		if (Q->CastOnTarget(minion))
		{
			return;
		}
	}
}

void Farm()
{
	if (LaneClearQ->Enabled() && Q->IsReady())
	{
		// add count to find best minion
		for (auto obj : GEntityList->GetAllUnits())
		{
			if (Utility::IsValidTarget(obj, Q->Range()) && IsPassiveActive(obj) && Q->CastOnUnit(obj))
			{
				return;
			}
		}
	}

	if (FarmW->Enabled() && W->IsReady() && IsFirstW())
	{
		Vec3 pos;
		int count;
		W->FindBestCastPosition(true, true, pos, count);

		if (count >= 4 && W->CastOnPosition(pos))
		{
			return;
		}
	}
}

bool Flee()
{
	if (!IsKeyDown(FleeKey))
	{
		return false;
	}

	if (Player->IsDashing())
	{
		return true;
	}

	auto pos = GGame->CursorPosition();

	GGame->IssueOrder(Player, kMoveTo, pos);

	if (FleeW->Enabled() && W->IsReady() && IsFirstW() && W->CastOnPosition(pos))
	{
		return true;
	}

	if (FleeRW->Enabled())
	{
		if (IsRActive() && W->IsReady() && W->CastOnPosition(pos))
		{
			return true;
		}

		return R->IsReady() && IsFirstR() && R->CastOnPlayer();
	}

	return false;
}

void KillSteal()
{
	auto q = Q->IsReady();
	auto w = W->IsReady() && IsFirstW();
	// check predicted pos
	auto e = E->IsReady();
	auto r = R->IsReady() && IsFirstR();
	auto range = W->Range() * 2;
	// this will bug when w => r(w)
	// pls fix

	for (auto unit : GEntityList->GetAllHeros(false, true))
	{
		if (!Utility::IsValidTarget(unit, range))
		{
			continue;
		}

		// w => r(w) => xxxx
		if (!Utility::IsValidTarget(unit, E->Range()))
		{
			// can't do anything
			if (!w || ! r)
			{
				return;
			}
			return;
		}

		auto canQ = q && !Utility::IsValidTarget(unit, Q->Range());
		// e => q
		if (!Utility::IsValidTarget(unit, W->Range()))
		{
			if (canQ)
			{
			}

			if (e)
			{
			}
			return;
		}
	}
}

PLUGIN_EVENT(void) OnGameUpdate()
{
	try
	{
		if (Player->IsDead())
		{
			return;
		}

		// auto e immobilize
		/*if (true && E->IsReady())
		{
			auto eRange = E->Range();
			for (auto enemy : GEntityList->GetAllHeros(false, true))
			{
				// check movement impaired
				if (!enemy->IsValidTarget() || (enemy->ServerPosition() - Player->ServerPosition()).Length() > eRange)
				{
					continue;
				}

				if (E->CastOnTarget(enemy))
				{
					break;
				}
			}
		}*/

		if (Flee())
		{
			return;
		}


		//ks and return
		if (true)
		{
		}

		switch (GOrbwalking->GetOrbwalkingMode())
		{
		case kModeCombo:
			Combo();
			break;
		case kModeLaneClear:
			Farm();
			break;
		case kModeLastHit:
			LastHit();
			break;
		case kModeMixed:
			Harass();
			break;
		}
	}
	catch (const std::exception& e)
	{
		GGame->PrintChat(e.what());
	}
}

PLUGIN_EVENT(void) OnRender()
{
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);

	MainMenu = GPluginSDK->AddMenu("PopBlanc");
	QMenu = MainMenu->AddMenu("Q");
	WMenu = MainMenu->AddMenu("W");
	EMenu = MainMenu->AddMenu("E");
	RMenu = MainMenu->AddMenu("R");
	FleeMenu = MainMenu->AddMenu("Flee");
	//Drawings = MainMenu->AddMenu("Drawings");

	ComboQ = QMenu->CheckBox("Use in Combo", true);
	HarassQ = QMenu->CheckBox("Use in Harass", true);
	HarassQBuff = QMenu->CheckBox("Harass Only w Buff", true);
	LastHitQ = QMenu->CheckBox("Use in LastHit", true);
	LaneClearQ = QMenu->CheckBox("Use in LaneClear", true);
	FarmQMana = QMenu->AddInteger("Farm Minimum Mana", 0, 100, 40);

	ComboW = WMenu->CheckBox("Use in Combo", true);
	HarassW = WMenu->CheckBox("Use in Harass", true);
	FarmW = WMenu->CheckBox("Use in LaneClear", true);

	ComboE = EMenu->CheckBox("Use E", true);

	ComboR = RMenu->CheckBox("Auto R", true);

	FleeKey = FleeMenu->AddKey("Flee", 'T');
	FleeW = FleeMenu->CheckBox("Use W", true);
	FleeRW = FleeMenu->CheckBox("Use R(W)", true);

	/*
	DrawReady = Drawings->CheckBox("Draw Only Ready Spells", true);

	DrawQ = Drawings->CheckBox("Draw Q", true);
	DrawW = Drawings->CheckBox("Draw W", true);
	DrawE = Drawings->CheckBox("Draw E", true);
	*/

	Q = GPluginSDK->CreateSpell2(kSlotQ, kTargetCast, false, false, static_cast<eCollisionFlags>(kCollidesWithYasuoWall));
	W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, false, true, static_cast<eCollisionFlags>(kCollidesWithYasuoWall));
	W->SetOverrideRange(880);
	E = GPluginSDK->CreateSpell2(kSlotE, kLineCast, true, false, static_cast<eCollisionFlags>(kCollidesWithHeroes | kCollidesWithMinions | kCollidesWithYasuoWall));
	R = GPluginSDK->CreateSpell2(kSlotR, kLineCast, false, false, static_cast<eCollisionFlags>(kCollidesWithYasuoWall));

	SpellMenuMap.insert(std::make_pair(Q, ComboQ));
	SpellMenuMap.insert(std::make_pair(W, ComboW));
	SpellMenuMap.insert(std::make_pair(E, ComboE));
	SpellMenuMap.insert(std::make_pair(R, ComboR));

	Player = GEntityList->Player();

	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);
}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
}

