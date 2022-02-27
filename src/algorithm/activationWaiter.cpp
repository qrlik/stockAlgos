#include "activationWaiter.h"
#include "moneyMaker.h"

using namespace algorithm;

activationWaiter::activationWaiter(moneyMaker* aMm, int aActivationWaitRange, bool aResetAllowed, bool aFullCandleCheck)
	:mm(aMm),
	activationWaitRange(aActivationWaitRange),
	resetAllowed(aResetAllowed),
	fullCandleCheck(aFullCandleCheck) {}

bool activationWaiter::operator==(const activationWaiter& other) {
	return activationWaitRange == other.activationWaitRange
		&& activationWaitCounter == other.activationWaitCounter
		&& resetAllowed == other.resetAllowed
		&& fullCandleCheck == other.fullCandleCheck;
}

int activationWaiter::getCounter() const {
	return activationWaitCounter;
}

void activationWaiter::setCounter(int aAmount) {
	activationWaitCounter = aAmount;
}

void activationWaiter::onNewTrend() {
	if (mm->getState() != moneyMaker::eState::ACTIVATION_WAIT) {
		return;
	}
	assert(activationWaitRange >= 0);
	if (resetAllowed) {
		mm->setState(moneyMaker::eState::NONE);
		activationWaitCounter = 0;
	}
	else {
		activationWaitCounter = activationWaitRange;
	}
}

void activationWaiter::start() {
	assert(activationWaitRange >= 0);
	mm->setState(moneyMaker::eState::ACTIVATION_WAIT);
	activationWaitCounter = activationWaitRange;
}

bool activationWaiter::check() {
	assert(activationWaitRange >= 0);
	const auto trendActivation = mm->getTrendActivation(mm->getActualSuperTrend());
	if (activationWaitCounter == 0) {
		const auto isTrendUp = mm->getIsTrendUp();
		const auto& open = mm->getCandle().open;
		if (isTrendUp && open > trendActivation) {
			mm->openOrder(moneyMaker::eState::LONG, open);
			return true;
		}
		else if (!isTrendUp && open < trendActivation) {
			mm->openOrder(moneyMaker::eState::SHORT, open);
			return true;
		}
		activationWaitCounter = activationWaitRange;
	}
	else {
		const auto& candle = mm->getCandle();
		auto minimum = (fullCandleCheck) ? candle.low : std::min(candle.open, candle.close);
		auto maximum = (fullCandleCheck) ? candle.high : std::max(candle.open, candle.close);
		const auto isTrendUp = mm->getIsTrendUp();
		if ((isTrendUp && minimum > trendActivation) || (!isTrendUp && maximum < trendActivation)) {
			activationWaitCounter -= 1;
		}
		else {
			activationWaitCounter = activationWaitRange;
		}
	}
	if (mm->isNewTrendChanged()) {
		activationWaitCounter = activationWaitRange;
	}
	return false;
}