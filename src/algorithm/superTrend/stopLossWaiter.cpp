#include "stopLossWaiter.h"
#include "stAlgorithm.h"

using namespace algorithm;

stopLossWaiter::stopLossWaiter(stAlgorithm* aMm, int aStopLossWaitRange, bool aEnabled, bool aResetAllowed, bool aFullCandleCheck):
	mm(aMm),
	stopLossWaitRange(aStopLossWaitRange),
	enabled(aEnabled),
	resetAllowed(aResetAllowed),
	fullCandleCheck(aFullCandleCheck) {}

bool stopLossWaiter::operator==(const stopLossWaiter& other) {
	return enabled == other.enabled
		&& stopLossWaitRange == other.stopLossWaitRange
		&& stopLossWaitCounter == other.stopLossWaitCounter
		&& resetAllowed == other.resetAllowed
		&& fullCandleCheck == other.fullCandleCheck;
}

int stopLossWaiter::getCounter() const {
	return stopLossWaitCounter;
}

void stopLossWaiter::setCounter(int aAmount) {
	stopLossWaitCounter = aAmount;
}

void stopLossWaiter::onNewTrend() {
	if (mm->getState() != eState::STOP_LOSS_WAIT) {
		return;
	}
	assert(stopLossWaitRange >= 0);
	if (resetAllowed) {
		mm->setState(eState::NONE);
		stopLossWaitCounter = 0;
	}
	else {
		stopLossWaitCounter = stopLossWaitRange;
	}
}

void stopLossWaiter::start() {
	if (enabled) {
		assert(stopLossWaitRange >= 0);
		mm->setState(eState::STOP_LOSS_WAIT);
		stopLossWaitCounter = stopLossWaitRange;
	}
	else {
		mm->setState(eState::NONE);
	}
}

bool stopLossWaiter::check() {
	if (!enabled) {
		mm->setState(eState::NONE);
		return true;
	}
	assert(stopLossWaitRange >= 0);
	const auto trendActivation = mm->getActualSuperTrend();
	if (stopLossWaitCounter == 0) {
		const auto isTrendUp = mm->getIsTrendUp();
		const auto& open = mm->getCandle().open;
		const bool trendUpAndOpenSuccess = isTrendUp && open > trendActivation;
		const bool trendDownAndOpenSuccess = !isTrendUp && open < trendActivation;
		if (trendUpAndOpenSuccess || trendDownAndOpenSuccess) {
			mm->setState(eState::NONE);
			return true;
		}
		stopLossWaitCounter = stopLossWaitRange;
	}
	else {
		const auto& candle = mm->getCandle();
		auto minimum = (fullCandleCheck) ? candle.low : std::min(candle.open, candle.close);
		auto maximum = (fullCandleCheck) ? candle.high : std::max(candle.open, candle.close);
		const auto isTrendUp = mm->getIsTrendUp();
		if ((isTrendUp && minimum > trendActivation) || (!isTrendUp && maximum < trendActivation)) {
			stopLossWaitCounter -= 1;
		}
		else {
			stopLossWaitCounter = stopLossWaitRange;
		}
	}
	if (mm->isNewTrendChanged()) {
		stopLossWaitCounter = stopLossWaitRange;
	}
	return false;
}