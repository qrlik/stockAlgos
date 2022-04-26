#include "trendBreakOpener.h"
#include "stAlgorithm.h"

using namespace algorithm;

namespace {
	double getOrderPrice(double aTrendActivation, double aOpen, eState aState) {
		if (aState == eState::LONG) {
			if (aOpen > aTrendActivation) {
				return aOpen;
			}
			return aTrendActivation;
		}
		else {
			if (aOpen < aTrendActivation) {
				return aOpen;
			}
			return aTrendActivation;
		}
	}
}

trendBreakOpener::trendBreakOpener(stAlgorithm& aAlgorithm):
	algorithm(aAlgorithm) {}

bool trendBreakOpener::isNewTrendAllowed() {
	if (!algorithm.getData().getBreakOpenerEnabled()) {
		return false;
	}
	if (algorithm.getData().getAlwaysUseNewTrend()) {
		return true;
	}
	const auto state = algorithm.getState();
	if (state != eState::LONG && state != eState::SHORT) {
		return true;
	}
	return false;
}

bool trendBreakOpener::check() {
	if (!algorithm.getData().getBreakOpenerEnabled()) {
		return false;
	}
	const auto trendActivation = algorithm.getActualSuperTrend();
	const auto isTrendUp = algorithm.getIsTrendUp();
	const auto& candle = algorithm.getCandle();
	if (isTrendUp && candle.high >= trendActivation) {
		if (!algorithm.getData().getBreakOpenerActivationWaitMode()) {
			const auto orderPrice = getOrderPrice(trendActivation, candle.open, eState::LONG);
			algorithm.openOrder(eState::LONG, orderPrice);
			return true;
		}
		algorithm.getActivationWaiter().start();
	}
	else if (!isTrendUp && candle.low <= trendActivation) {
		if (!algorithm.getData().getBreakOpenerActivationWaitMode()) {
			const auto orderPrice = getOrderPrice(trendActivation, candle.open, eState::SHORT);
			algorithm.openOrder(eState::SHORT, orderPrice);
			return true;
		}
		algorithm.getActivationWaiter().start();
	}
	if (algorithm.isNewTrendChanged()) {
		return true;
	}
	return false;
}