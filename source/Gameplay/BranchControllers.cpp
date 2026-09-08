#include "BranchControllers.h"

#include <CTheScripts.h>
#include <memory>
#include <vector>

#include "BranchProgress.h"
#include "Branches/EdgeCases.h"
#include "RunningScripts.h"

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

		return controllers;
	}

	void startScriptIfNeeded(const BranchController& t_controller)
	{
		if (RunningScripts::isActive(t_controller.scriptName())) return;

		CTheScripts::StartNewScript(
			reinterpret_cast<unsigned char*>(CTheScripts::ScriptSpace) + t_controller.address());
	}
}

void BranchControllers::update(const BranchProgress& t_progress)
{
	static std::vector<std::unique_ptr<BranchController>> controllers = makeControllers();

	for (const std::unique_ptr<BranchController>& controller : controllers)
	{
		EdgeCase* edge = controller->asEdgeCase();

		if (edge) edge->update();

		if (controller->finished()) continue;
		if (!controller->gateOpen(t_progress)) continue;

		startScriptIfNeeded(*controller);

		if (!edge) controller->defaultMarker().raise();
	}
}
