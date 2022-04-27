#include "stopLossWaiter.h"
#include "stAlgorithm.h"
#include "utils/utils.h"

using namespace algorithm;

stopLossWaiter::stopLossWaiter(stAlgorithm& aAlgorithm):
	algorithm(aAlgorithm) {}

bool stopLossWaiter::operator==(const stopLossWaiter& aOther) const {
	return stopLossWaitCounter == aOther.stopLossWaitCounter;
}

int stopLossWaiter::getCounter() const {
	return stopLossWaitCounter;
}

void stopLossWaiter::onNewTrend() {
	if (algorithm.getState() != eState::STOP_LOSS_WAIT) {
		return;
	}
	if (algorithm.getData().getStopLossWaiterResetAllowed()) {
		algorithm.setState(eState::NONE);
		stopLossWaitCounter = 0;
	}
	else {
		stopLossWaitCounter = algorithm.getData().getStopLossWaiterRange();
	}
}

void stopLossWaiter::start() {
	if (algorithm.getData().getStopLossWaiterEnabled()) {
		if (algorithm.getData().getStopLossWaiterRange() >= 0) {
			algorithm.setState(eState::STOP_LOSS_WAIT);
			stopLossWaitCounter = algorithm.getData().getStopLossWaiterRange();
		}
		else {
			utils::logError("stopLossWaiter::start wrong range");
			algorithm.setState(eState::NONE);
		}
	}
	else {
		algorithm.setState(eState::NONE);
	}
}

bool stopLossWaiter::check() {
	if (algorithm.getState() != eState::STOP_LOSS_WAIT) {
		return false;
	}
	const auto trendActivation = algorithm.getActualSuperTrend();
	if (stopLossWaitCounter == 0) {
		const auto isTrendUp = algorithm.getIsTrendUp();
		const auto& open = algorithm.getCandle().open;
		const bool trendUpAndOpenSuccess = isTrendUp && open > trendActivation;
		const bool trendDownAndOpenSuccess = !isTrendUp && open < trendActivation;
		if (trendUpAndOpenSuccess || trendDownAndOpenSuccess) {
			algorithm.setState(eState::NONE);
			return true;
		}
		stopLossWaitCounter = algorithm.getData().getStopLossWaiterRange();
	}
	else {
		const auto& candle = algorithm.getCandle();
		const auto fullCandleCheck = algorithm.getData().getStopLossWaiterFullCandleCheck();
		auto minimum = (fullCandleCheck) ? candle.low : std::min(candle.open, candle.close);
		auto maximum = (fullCandleCheck) ? candle.high : std::max(candle.open, candle.close);
		const auto isTrendUp = algorithm.getIsTrendUp();
		if ((isTrendUp && minimum > trendActivation) || (!isTrendUp && maximum < trendActivation)) {
			stopLossWaitCounter -= 1;
		}
		else {
			stopLossWaitCounter = algorithm.getData().getStopLossWaiterRange();
		}
	}
	if (algorithm.isNewTrendChanged()) {
		stopLossWaitCounter = algorithm.getData().getStopLossWaiterRange();
	}
	return false;
}