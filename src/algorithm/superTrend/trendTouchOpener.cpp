#include "trendTouchOpener.h"
#include "stAlgorithm.h"

using namespace algorithm;

namespace {
	double getOrderPrice(double aTrendActivation, double aOpen, eState aState) {
		if (aState == eState::LONG) {
			if (aOpen < aTrendActivation) {
				return aOpen;
			}
			return aTrendActivation;
		}
		else {
			if (aOpen > aTrendActivation) {
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
	if (isTrendUp && candle.low <= trendActivation) {
		if (!algorithm.getData().getTouchOpenerActivationWaitMode()) {
			const auto orderPrice = getOrderPrice(trendActivation, candle.open, eState::LONG);
			algorithm.openOrder(eOrderState::LONG, orderPrice);
			return true;
		}
		algorithm.getActivationWaiter().start();
	}
	else if (!isTrendUp && candle.high >= trendActivation) {
		if (!algorithm.getData().getTouchOpenerActivationWaitMode()) {
			const auto orderPrice = getOrderPrice(trendActivation, candle.open, eState::SHORT);
			algorithm.openOrder(eOrderState::SHORT, orderPrice);
			return true;
		}
		algorithm.getActivationWaiter().start();
	}
	return false;
}