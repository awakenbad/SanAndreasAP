#include "BranchControllers.h"
#include "Branches/EdgeCases.h"
#include "Branches/OutOfOrderGuards.h"
#include "ScriptCommandHook.h"
#include "BranchProgress.h"
#include "CTheScripts.h"
#include <eScriptCommands.h>
#include <memory>
#include <vector>

namespace
{
	constexpr int LEAVE = Marker::LEAVE_DISPLAY;
	constexpr int BLIP_ONLY = Marker::BLIP_ONLY;
	constexpr int NONE = BranchController::NO_PREREQUISITE;

	constexpr BranchRow LINEAR_BRANCHES[] = {
		{ "SWEET", 62090, 1808, 9, 1848, 1768, 1740, LEAVE, NONE },
		{ "RYDER", 63143, 1812, 3, 1860, 1772, 1744, LEAVE, 12 },
		{ "SMOKE", 63492, 1816, 4, 1872, 1780, 1736, LEAVE, NONE },
		{ "STRAP", 63835, 1820, 5, 1884, 1784, 1748, LEAVE, NONE },
		{ "CRASH", 62904, 1824, 2, 1908, 1776, 1752, LEAVE, NONE },
		{ "CESAR", 64462, 1828, 1, 1920, 1788, 1760, BLIP_ONLY, NONE },
	};

	std::vector<std::unique_ptr<BranchController>> makeControllers()
	{
		std::vector<std::unique_ptr<BranchController>> controllers;

		for (const BranchRow& row : LINEAR_BRANCHES)
		{
			controllers.push_back(std::make_unique<BranchController>(row));
		}

		controllers.push_back(std::make_unique<BcrashController>());
		controllers.push_back(std::make_unique<CatController>());
		controllers.push_back(std::make_unique<TruController>());
		controllers.push_back(std::make_unique<BcesarController>());
		controllers.push_back(std::make_unique<GarageController>());
		controllers.push_back(std::make_unique<WuziController>());
		controllers.push_back(std::make_unique<TorenoController>());
		controllers.push_back(std::make_unique<CasinoController>());
		controllers.push_back(std::make_unique<VegasCrashController>());
		controllers.push_back(std::make_unique<MaddDoggController>());

		return controllers;
	}

	std::vector<std::unique_ptr<BranchController>>& controllers()
	{
		static std::vector<std::unique_ptr<BranchController>> all = makeControllers();
		return all;
	}

	void startScriptIfNeeded(const BranchController& t_controller)
	{
		if (t_controller.running()) return;

		CTheScripts::StartNewScript(
			reinterpret_cast<unsigned char*>(CTheScripts::ScriptSpace) + t_controller.address());
	}

	constexpr unsigned char END_OF_ARGUMENTS = 0;

	bool skipDuplicateStart(CRunningScript* t_script)
	{
		t_script->CollectParameters(1);
		int target = ScriptParams[0];

		if (*t_script->m_pCurrentIP != END_OF_ARGUMENTS) return false;

		for (const std::unique_ptr<BranchController>& controller : controllers())
		{
			if (controller->address() != target) continue;
			if (!controller->running()) return false;

			++t_script->m_pCurrentIP;
			return true;
		}
		return false;
	}

	void installHooks()
	{
		static bool installed = false;
		if (installed) return;
		installed = true;

		ScriptCommandHook::replaceCommand(COMMAND_START_NEW_SCRIPT, &skipDuplicateStart);
		OutOfOrderGuards::install();
	}
}

void BranchControllers::update(const BranchProgress& t_progress)
{
	installHooks();

	for (const std::unique_ptr<BranchController>& controller : controllers())
	{
		EdgeCase* edge = controller->asEdgeCase();

		if (edge) edge->update();

		if (controller->finished()) continue;
		if (!controller->gateOpen(t_progress)) continue;

		startScriptIfNeeded(*controller);

		if (!edge) controller->defaultMarker().raise();
	}

	OutOfOrderGuards::update();
}
