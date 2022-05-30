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
			return true;
		}

		if (checkStates(true)) {
			algorithm.closeOrder(algorithm.getCandle().open);
			return true;
		}
	}
	else if (algorithm.getState() == getIntState(eBaseState::SHORT)) {
		if (utils::isGreaterOrEqual(algorithm.getCandle().high, algorithm.getOrder().getStopLoss())) {
			algorithm.closeOrder();
			return true;
		}

		if (checkStates(false)) {
			algorithm.closeOrder(algorithm.getCandle().open);
			return true;
		}
	}
	return false;
}

void closerModule::updateState(bool& aState, bool aAdd) const {
	if (algorithm.getData().getCloserConjuctionCheck()) {
		aState &= aAdd;
	}
	else {
		aState |= aAdd;
	}
}

bool closerModule::checkStates(bool aLong) const {
	bool checkState = algorithm.getData().getCloserConjuctionCheck();
	if (algorithm.getData().getCloserTrendChangeCheck()) {
		auto trendState = algorithm.getIndicators().isSuperTrendUp();
		trendState = (aLong) ? !trendState : trendState;
		updateState(checkState, trendState);
	}
	if (algorithm.getData().getCloserMACheck()) {
		auto mainMAState = algorithm.getMAModule().isSecondUp();
		mainMAState = (aLong) ? !mainMAState : mainMAState;
		updateState(checkState, mainMAState);
	}
	return checkState;
}