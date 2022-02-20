#include "stopLossWaiter.h"
#include "moneyMaker.h"

using namespace algorithm;

stopLossWaiter::stopLossWaiter(moneyMaker* aMm, size_t aStopLossWaitRange, bool aEnabled, bool aResetAllowed, bool aFullCandleCheck):
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

size_t stopLossWaiter::getCounter() const {
	return stopLossWaitCounter;
}

void stopLossWaiter::onNewTrend() {
	if (mm->getState() != moneyMaker::eState::STOP_LOSS_WAIT) {
		return;
	}
	if (resetAllowed) {
		mm->setState(moneyMaker::eState::NONE);
		stopLossWaitCounter = 0;
	}
	else {
		stopLossWaitCounter = stopLossWaitRange;
	}
}

void stopLossWaiter::start() {
	if (enabled) {
		mm->setState(moneyMaker::eState::STOP_LOSS_WAIT);
		stopLossWaitCounter = stopLossWaitRange;
	}
	else {
		mm->setState(moneyMaker::eState::NONE);
	}
}

bool stopLossWaiter::check() {
	if (!enabled) {
		mm->setState(moneyMaker::eState::NONE);
		return true;
	}
	const auto trendActivation = mm->getTrendActivation(mm->getActualSuperTrend());
	if (stopLossWaitCounter == 0) {
		const auto isTrendUp = mm->getIsTrendUp();
		const auto& open = mm->getCandle().open;
		const bool trendUpAndOpenSuccess = isTrendUp && open > trendActivation;
		const bool trendDownAndOpenSuccess = !isTrendUp && open < trendActivation;
		if (trendUpAndOpenSuccess || trendDownAndOpenSuccess) {
			mm->setState(moneyMaker::eState::NONE);
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