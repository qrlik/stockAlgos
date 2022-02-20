#include "trendTouchOpener.h"
#include "moneyMaker.h"

using namespace algorithm;

namespace {
	double getOrderPrice(double aTrendActivation, double aOpen, moneyMaker::eState aState) {
		if (aState == moneyMaker::eState::LONG) {
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

trendTouchOpener::trendTouchOpener(moneyMaker* aMm, bool aActivationWaitMode) :
	mm(aMm),
	activationWaitMode(aActivationWaitMode) {}

bool trendTouchOpener::check() {
	const auto trendActivation = mm->getTrendActivation(mm->getSuperTrend());
	const auto isTrendUp = mm->getIsTrendUp();
	const auto& candle = mm->getCandle();
	if (isTrendUp && candle.low <= trendActivation) {
		if (!activationWaitMode) {
			const auto orderPrice = getOrderPrice(trendActivation, candle.open, moneyMaker::eState::LONG);
			mm->openOrder(moneyMaker::eState::LONG, orderPrice);
			return true;
		}
		mm->getActivationWaiter().start();
	}
	else if (!isTrendUp && candle.high >= trendActivation) {
		if (!activationWaitMode) {
			const auto orderPrice = getOrderPrice(trendActivation, candle.open, moneyMaker::eState::SHORT);
			mm->openOrder(moneyMaker::eState::SHORT, orderPrice);
			return true;
		}
		mm->getActivationWaiter().start();
	}
	return false;
}