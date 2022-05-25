#include "openerModule.h"
#include "stMAlgorithm.h"

using namespace algorithm;

openerModule::openerModule(stMAlgorithm& aAlgorithm)
	:algorithm(aAlgorithm) {}

bool openerModule::check() {
	const auto isFirstMAGrowing = algorithm.getMAModule().isFirstUp();
	const auto isSecondMAGrowing = algorithm.getMAModule().isSecondUp();
	const auto firstMA = algorithm.getIndicators().getFirstMA();
	const auto secondMA = algorithm.getIndicators().getSecondMA();
	if (algorithm.getIndicators().isSuperTrendUp()) {
		if (isFirstMAGrowing && isSecondMAGrowing) {
			if (utils::isGreater(secondMA, firstMA)) {
				algorithm.openOrder(eOrderState::LONG, algorithm.getCandle().open);
				return true;
			}
		}
	}
	else {
		if (!isFirstMAGrowing && !isSecondMAGrowing) {
			if (utils::isGreater(firstMA, secondMA)) {
				algorithm.openOrder(eOrderState::SHORT, algorithm.getCandle().open);
				return true;
			}
		}
	}
	return false;
}