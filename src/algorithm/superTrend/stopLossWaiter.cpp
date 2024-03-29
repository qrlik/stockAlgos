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
	if (algorithm.getState() != getIntState(eCustomState::STOP_LOSS_WAIT)) {
		return;
	}
	if (algorithm.getData().getStopLossWaiterResetAllowed()) {
		algorithm.setState(getIntState(eBaseState::NONE));
		stopLossWaitCounter = 0;
	}
	else {
		stopLossWaitCounter = algorithm.getData().getStopLossWaiterRange();
	}
}

void stopLossWaiter::start() {
	if (algorithm.getData().getStopLossWaiterEnabled()) {
		if (algorithm.getData().getStopLossWaiterRange() >= 0) {
			algorithm.setState(getIntState(eCustomState::STOP_LOSS_WAIT));
			stopLossWaitCounter = algorithm.getData().getStopLossWaiterRange();
		}
		else {
			utils::logError("stopLossWaiter::start wrong range");
			algorithm.setState(getIntState(eBaseState::NONE));
		}
	}
	else {
		algorithm.setState(getIntState(eBaseState::NONE));
	}
}

bool stopLossWaiter::check() {
	if (algorithm.getState() != getIntState(eCustomState::STOP_LOSS_WAIT)) {
		return false;
	}
	const auto trendActivation = algorithm.getActualSuperTrend();
	if (stopLossWaitCounter == 0) {
		const auto isTrendUp = algorithm.getIsTrendUp();
		const auto& open = algorithm.getCandle().open;
		const bool trendUpAndOpenSuccess = isTrendUp && utils::isGreater(open, trendActivation);
		const bool trendDownAndOpenSuccess = !isTrendUp && utils::isLess(open, trendActivation);
		if (trendUpAndOpenSuccess || trendDownAndOpenSuccess) {
			algorithm.setState(getIntState(eBaseState::NONE));
			return true;
		}
		stopLossWaitCounter = algorithm.getData().getStopLossWaiterRange();
	}
	else {
		const auto& candle = algorithm.getCandle();
		const auto fullCandleCheck = algorithm.getData().getStopLossWaiterFullCandleCheck();
		auto minimum = (fullCandleCheck) ? candle.low : utils::minFloat(candle.open, candle.close);
		auto maximum = (fullCandleCheck) ? candle.high : utils::maxFloat(candle.open, candle.close);
		const auto isTrendUp = algorithm.getIsTrendUp();
		if ((isTrendUp && utils::isGreater(minimum, trendActivation)) || (!isTrendUp && utils::isLess(maximum, trendActivation))) {
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