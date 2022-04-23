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

trendBreakOpener::trendBreakOpener(stAlgorithm* aMm, bool aEnabled, bool aActivationWaitMode, bool aAlwaysUseNewTrend):
	mm(aMm),
	enabled(aEnabled),
	activationWaitMode(aActivationWaitMode),
	alwaysUseNewTrend(aAlwaysUseNewTrend) {}

bool trendBreakOpener::isNewTrendAllowed() {
	if (!enabled) {
		return false;
	}
	if (alwaysUseNewTrend) {
		return true;
	}
	const auto state = mm->getState();
	if (state != eState::LONG && state != eState::SHORT) {
		return true;
	}
	return false;
}

bool trendBreakOpener::check() {
	const auto trendActivation = mm->getActualSuperTrend();
	const auto isTrendUp = mm->getIsTrendUp();
	const auto& candle = mm->getCandle();
	if (isTrendUp && candle.high >= trendActivation) {
		if (!activationWaitMode) {
			const auto orderPrice = getOrderPrice(trendActivation, candle.open, eState::LONG);
			mm->openOrder(eState::LONG, orderPrice);
			return true;
		}
		mm->getActivationWaiter().start();
	}
	else if (!isTrendUp && candle.low <= trendActivation) {
		if (!activationWaitMode) {
			const auto orderPrice = getOrderPrice(trendActivation, candle.open, eState::SHORT);
			mm->openOrder(eState::SHORT, orderPrice);
			return true;
		}
		mm->getActivationWaiter().start();
	}
	if (mm->isNewTrendChanged()) {
		return true;
	}
	return false;
}