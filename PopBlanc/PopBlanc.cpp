#include <map>
#include "PluginSDK.h"
#include "Utility.h"
PluginSetup("PopBlanc - Trees");

IMenu* MainMenu;
IMenu* QMenu;
IMenu* WMenu;
IMenu* EMenu;
IMenu* RMenu;
IMenu* Misc;
IMenu* Drawings;

IMenuOption* ComboQ;
IMenuOption* HarassQ;
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


IUnit* GetTarget()
{
	/*auto eRange = E->Range();

	for (auto enemy : GEntityList->GetAllHeros(false, true))
	{
	if (enemy == nullptr || !enemy->IsValidTarget(Player, eRange))
	{
	continue;
	}

	if (enemy->HasBuff(QBuff) || enemy->HasBuff(EBuff))
	{
	return enemy;
	}
	}*/

	return GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->IsReady() ? E->Range() : W->Range());
}


bool bCastSpell(ISpell2* spell, IUnit* target)
{
	auto opt = SpellMenuMap[spell];
	return opt != nullptr && opt->Enabled() && spell->IsReady() && target->IsValidTarget(Player, spell->Range());
}

bool IsFirstW()
{
	return strcmp(Player->GetSpellBook()->GetName(kSlotW), "LeblancW") == 0;
}

bool IsFirstR()
{
	return strcmp(Player->GetSpellBook()->GetName(kSlotR), "LeblancRToggle") == 0;
}

bool CastSecondW()
{
	// Second W Enabled
	return GOrbwalking->GetOrbwalkingMode() == kModeMixed && W->IsReady() && !Q->IsReady() && !IsFirstW() && W->CastOnPlayer();
}

bool CastRSpell(IUnit* target)
{
	if (strcmp(Player->GetSpellBook()->GetName(kSlotR), "LeblancR") != 0)
	{
		return false;
	}

	if (Utility::CountBuffs(Utility::GetEnemiesInRange(target, 500), QBuff) > 0)
	{
		if (bCastSpell(W, target) && IsFirstW() && W->CastOnTargetAoE(target))
		{
			return true;
		}

		auto hasBuff = target->HasBuff(QBuff);
		if ((target->GetHealth() < GDamage->GetSpellDamage(Player, target, kSlotQ, hasBuff ? 1 : 0) || hasBuff) && bCastSpell(Q, target) && Q->CastOnTarget(target))
		{
			return true;
		}
	}

	return bCastSpell(E, target) && E->CastOnUnit(target);
}

void Combo(IUnit* targ = nullptr, bool force = false)
{
	try
	{
		auto target = targ == nullptr ? GetTarget() : targ;

		if (target == nullptr || !target->IsValidTarget())
		{
			return;
		}

		if (CastRSpell(target))
		{
			return;
		}

		if (bCastSpell(W, target) && IsFirstW() && W->CastOnUnit(target))
		{
			return;
		}

		if (bCastSpell(E, target) && E->CastOnUnit(target))
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
	if (!HarassQ->Enabled() || !HarassW->Enabled() || !W->IsReady() || !IsFirstW() || !Q->IsReady())
	{
		return;
	}

	auto target = GetTarget();

	if (target == nullptr || !target->IsValidTarget(Player, W->Range()))
	{
		return;
	}

	if (W->CastOnUnit(target))
	{
		if (target->IsValidTarget(Player, Q->Range()) && Q->CastOnUnit(target))
		{
			return;
		}

		for (auto obj: GEntityList->GetAllUnits())
		{
			if (obj != nullptr && obj->IsValidTarget(Player, Q->Range()) && obj->HasBuff(QBuff) && (obj->ServerPosition() - target->ServerPosition()).Length() < 500 && Q->CastOnUnit(obj))
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
		if (minion == nullptr || !minion->IsValidTarget(Player, Q->Range()) || minion->GetHealth() > GDamage->GetSpellDamage(Player, minion, kSlotQ))
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
	if (HarassW->Enabled() && W->IsReady())
	{
	}

	// if not farm
	if (false || (!Q->IsReady() && !W->IsReady()))
	{
		return;
	}

	for (auto minion : GEntityList->GetAllMinions(false, true, false))
	{
		if (minion == nullptr || !minion->IsValidTarget(Player, Q->Range()))
		{
			continue;
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

		// flee then return
		if (true)
		{
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
	//Drawings = MainMenu->AddMenu("Drawings");

	ComboQ = QMenu->CheckBox("Use in Combo", true);
	HarassQ = QMenu->CheckBox("Use in Harass", true);
	LastHitQ = QMenu->CheckBox("Use in LastHit", true);
	LaneClearQ = QMenu->CheckBox("Use in LaneClear", true);
	FarmQMana = QMenu->AddInteger("Farm Minimum Mana", 0, 100, 40);

	ComboW = WMenu->CheckBox("Use in Combo", true);
	HarassW = WMenu->CheckBox("Use in Harass", true);
	FarmW = WMenu->CheckBox("Use in LaneClear", true);

	ComboE = EMenu->CheckBox("Use E", true);


	ComboR = RMenu->CheckBox("Auto R", true);

	/*
	DrawReady = Drawings->CheckBox("Draw Only Ready Spells", true);

	DrawQ = Drawings->CheckBox("Draw Q", true);
	DrawW = Drawings->CheckBox("Draw W", true);
	DrawE = Drawings->CheckBox("Draw E", true);
	*/

	Q = GPluginSDK->CreateSpell2(kSlotQ, kTargetCast, false, false, static_cast<eCollisionFlags>(kCollidesWithYasuoWall));
	W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, false, true, static_cast<eCollisionFlags>(kCollidesWithYasuoWall));
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

