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

trendTouchOpener::trendTouchOpener(stAlgorithm* aMm, bool aActivationWaitMode) :
	mm(aMm),
	activationWaitMode(aActivationWaitMode) {}

bool trendTouchOpener::check() {
	const auto trendActivation = mm->getSuperTrend();
	const auto isTrendUp = mm->getIsTrendUp();
	const auto& candle = mm->getCandle();
	if (isTrendUp && candle.low <= trendActivation) {
		if (!activationWaitMode) {
			const auto orderPrice = getOrderPrice(trendActivation, candle.open, eState::LONG);
			mm->openOrder(eState::LONG, orderPrice);
			return true;
		}
		mm->getActivationWaiter().start();
	}
	else if (!isTrendUp && candle.high >= trendActivation) {
		if (!activationWaitMode) {
			const auto orderPrice = getOrderPrice(trendActivation, candle.open, eState::SHORT);
			mm->openOrder(eState::SHORT, orderPrice);
			return true;
		}
		mm->getActivationWaiter().start();
	}
	return false;
}