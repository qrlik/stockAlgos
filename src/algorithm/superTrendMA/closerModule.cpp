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
		if (isNeedToClose(true)) {
			algorithm.closeOrder(algorithm.getCandle().open);
			return true;
		}

		if (utils::isLessOrEqual(algorithm.getCandle().low, getOrder().getStopLoss())) {
			algorithm.closeOrder();
			return true;
		}
	}
	else if (algorithm.getState() == getIntState(eBaseState::SHORT)) {
		if (isNeedToClose(false)) {
			algorithm.closeOrder(algorithm.getCandle().open);
			return true;
		}

		if (utils::isGreaterOrEqual(algorithm.getCandle().high, getOrder().getStopLoss())) {
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

bool closerModule::isNeedToClose(bool aLong) const {
	bool checkState = algorithm.getData().getCloserConjuctionCheck();
	if (algorithm.getData().getCloserTrendChangeCheck()) {
		auto trendState = algorithm.getIndicators().isSuperTrendUp();
		trendState = (aLong) ? !trendState : trendState;
		updateState(checkState, trendState);
	}
	if (algorithm.getData().getCloserMACheck()) {
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
	// if touched this candle -> return
	// else close < trail if long / close > trail if short
	const auto pricePrecision = algorithm.getData().getMarketData().getPricePrecision(algorithm.getCandle().high);
	if (algorithm.getState() == getIntState(eBaseState::LONG)) {
		auto trailStopLoss = utils::round(algorithm.getCandle().high * (100.0 - algorithm.getData().getCloserTrailPrecision()) / 100.0, pricePrecision);
		if (utils::isGreaterOrEqual(trailStopLoss, getOrder().getMinimumProfit()) && utils::isGreater(trailStopLoss, getOrder().getStopLoss())) {
			algorithm.updateOrderStopLoss(trailStopLoss);
			return true;
		}
	}
	else if (algorithm.getState() == getIntState(eBaseState::SHORT)) {
		auto trailStopLoss = utils::round(algorithm.getCandle().low * (100.0 + algorithm.getData().getCloserTrailPrecision()) / 100.0, pricePrecision);
		if (utils::isLessOrEqual(trailStopLoss, getOrder().getMinimumProfit()) && utils::isLess(trailStopLoss, getOrder().getStopLoss())) {
			algorithm.updateOrderStopLoss(trailStopLoss);
			return true;
		}
	}
	return false;
}