#include "closerModule.h"
#include "stMAlgorithm.h"
#include "utils/utils.h"

using namespace algorithm;

closerModule::closerModule(stMAlgorithm& aAlgorithm)
	:algorithm(aAlgorithm) {}


const order& closerModule::getOrder() const {
	return static_cast<const stMAlgorithm&>(algorithm).getOrder();
}

bool closerModule::check() {
	if (algorithm.getState() == getIntState(eBaseState::LONG)) {
		if (!algorithm.getIndicators().isSuperTrendUp()) {
			algorithm.closeOrder(algorithm.getCandle().open);
			return true;
		}
		if (utils::isLessOrEqual(algorithm.getCandle().low, getOrder().getStopLoss())) {
			algorithm.closeOrder();
			return true;
		}
	}
	else if (algorithm.getState() == getIntState(eBaseState::SHORT)) {
		if (algorithm.getIndicators().isSuperTrendUp()) {
			algorithm.closeOrder(algorithm.getCandle().open);
			return true;
		}
		if (utils::isGreaterOrEqual(algorithm.getCandle().high, getOrder().getStopLoss())) {
			algorithm.closeOrder();
			return true;
		}
	}
	return false;
}
