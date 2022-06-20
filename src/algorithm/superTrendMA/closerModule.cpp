#include "closerModule.h"
#include "stMAlgorithm.h"
#include "utils/utils.h"

using namespace algorithm;

closerModule::closerModule(stMAlgorithm& aAlgorithm)
	:algorithm(aAlgorithm) {}


bool closerModule::check() {
	if (algorithm.getState() == getIntState(eBaseState::LONG)) {
		if (checkStates(true)) {
			algorithm.closeOrder(algorithm.getCandle().open);
			return true;
		}

		if (utils::isLessOrEqual(algorithm.getCandle().low, algorithm.getOrder().getStopLoss())) {
			algorithm.closeOrder();
			return true;
		}
	}
	else if (algorithm.getState() == getIntState(eBaseState::SHORT)) {
		if (checkStates(false)) {
			algorithm.closeOrder(algorithm.getCandle().open);
			return true;
		}

		if (utils::isGreaterOrEqual(algorithm.getCandle().high, algorithm.getOrder().getStopLoss())) {
			algorithm.closeOrder();
			return true;
		}
	}
	return updateTrail();
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
	if (algorithm.getData().getCloserMACheck() && !algorithm.getMAModule().isCloserDiscrepancy()) {
		auto mainMAState = algorithm.getMAModule().isCloserUp();
		mainMAState = (aLong) ? !mainMAState : mainMAState;
		updateState(checkState, mainMAState);
	}
	return checkState;
}

bool closerModule::updateTrail() {
	if (!algorithm.getData().getCloserTrailStop()) {
		return false;
	}
	const auto pricePrecision = MARKET_DATA->getPricePrecision();
	if (algorithm.getState() == getIntState(eBaseState::LONG)) {
		auto trailStopLoss = utils::round(algorithm.getCandle().high * (100.0 - algorithm.getData().getCloserTrailPrecision()) / 100.0, pricePrecision);
		if (utils::isGreaterOrEqual(trailStopLoss, algorithm.getOrder().getMinimumProfit()) && utils::isGreater(trailStopLoss, algorithm.getOrder().getStopLoss())) {
			algorithm.updateOrderStopLoss(trailStopLoss);
			return true;
		}
	}
	else if (algorithm.getState() == getIntState(eBaseState::SHORT)) {
		auto trailStopLoss = utils::round(algorithm.getCandle().low * (100.0 + algorithm.getData().getCloserTrailPrecision()) / 100.0, pricePrecision);
		if (utils::isLessOrEqual(trailStopLoss, algorithm.getOrder().getMinimumProfit()) && utils::isLess(trailStopLoss, algorithm.getOrder().getStopLoss())) {
			algorithm.updateOrderStopLoss(trailStopLoss);
			return true;
		}
	}
	return false;
}