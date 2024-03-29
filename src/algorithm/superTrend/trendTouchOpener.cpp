#include "trendTouchOpener.h"
#include "stAlgorithm.h"

using namespace algorithm;

namespace {
	double getOrderPrice(double aTrendActivation, double aOpen, eOrderState aState) {
		if (aState == eOrderState::LONG) {
			if (utils::isLess(aOpen, aTrendActivation)) {
				return aOpen;
			}
			return aTrendActivation;
		}
		else {
			if (utils::isGreater(aOpen, aTrendActivation)) {
				return aOpen;
			}
			return aTrendActivation;
		}
	}
}

trendTouchOpener::trendTouchOpener(stAlgorithm& aAlgorithm) :
	algorithm(aAlgorithm) {}

bool trendTouchOpener::check() {
	const auto trendActivation = algorithm.getSuperTrend();
	const auto isTrendUp = algorithm.getIsTrendUp();
	const auto& candle = algorithm.getCandle();
	if (isTrendUp && utils::isLessOrEqual(candle.low, trendActivation)) {
		if (!algorithm.getData().getTouchOpenerActivationWaitMode()) {
			const auto orderPrice = getOrderPrice(trendActivation, candle.open, eOrderState::LONG);
			algorithm.openOrder(eOrderState::LONG, orderPrice);
			return true;
		}
		algorithm.getActivationWaiter().start();
	}
	else if (!isTrendUp && utils::isGreaterOrEqual(candle.high, trendActivation)) {
		if (!algorithm.getData().getTouchOpenerActivationWaitMode()) {
			const auto orderPrice = getOrderPrice(trendActivation, candle.open, eOrderState::SHORT);
			algorithm.openOrder(eOrderState::SHORT, orderPrice);
			return true;
		}
		algorithm.getActivationWaiter().start();
	}
	return false;
}