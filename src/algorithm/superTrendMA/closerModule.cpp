#include "closerModule.h"
#include "stMAlgorithm.h"
#include "utils/utils.h"

using namespace algorithm;

closerModule::closerModule(stMAlgorithm& aAlgorithm)
	:algorithm(aAlgorithm) {}

bool closerModule::check() {
	if (algorithm.getState() == getIntState(eBaseState::LONG)) {
		if (utils::isLessOrEqual(algorithm.getCandle().low, algorithm.getOrder().getStopLoss())) {
			algorithm.closeOrder();
		}
	}
	else if (algorithm.getState() == getIntState(eBaseState::SHORT)) {
		if (utils::isGreaterOrEqual(algorithm.getCandle().high, algorithm.getOrder().getStopLoss())) {
			algorithm.closeOrder();
		}
	}
	return false;
}