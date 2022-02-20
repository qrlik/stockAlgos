#include "trendBreakOpener.h"
#include "moneyMaker.h"

using namespace algorithm;

namespace {
	double getOrderPrice(double aTrendActivation, double aOpen, moneyMaker::eState aState) {
		if (aState == moneyMaker::eState::LONG) {
			if (aOpen > aTrendActivation) {
				return aOpen;
			}
			return aTrendActivation;
		}
		else if (aState == moneyMaker::eState::SHORT) {
			if (aOpen < aTrendActivation) {
				return aOpen;
			}
			return aTrendActivation;
		}
	}
}

trendBreakOpener::trendBreakOpener(moneyMaker* aMm, bool aEnabled, bool aActivationWaitMode, bool aAlwaysUseNewTrend):
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
	if (state != moneyMaker::eState::LONG && state != moneyMaker::eState::SHORT) {
		return true;
	}
	return false;
}

bool trendBreakOpener::check() {
	const auto trendActivation = mm->getTrendActivation(mm->getActualSuperTrend());
	const auto isTrendUp = mm->getIsTrendUp();
	const auto& candle = mm->getCandle();
	if (isTrendUp && candle.high >= trendActivation) {
		if (!activationWaitMode) {
			const auto orderPrice = getOrderPrice(trendActivation, candle.open, moneyMaker::eState::LONG);
			mm->openOrder(moneyMaker::eState::LONG, orderPrice);
			return true;
		}
		mm->getActivationWaiter().start();
	}
	else if (!isTrendUp && candle.low <= trendActivation) {
		if (!activationWaitMode) {
			const auto orderPrice = getOrderPrice(trendActivation, candle.open, moneyMaker::eState::SHORT);
			mm->openOrder(moneyMaker::eState::SHORT, orderPrice);
			return true;
		}
		mm->getActivationWaiter().start();
	}
	if (mm->isNewTrendChanged()) {
		return true;
	}
	return false;
}