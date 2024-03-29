#include "activationWaiter.h"
#include "stAlgorithm.h"
#include "utils/utils.h"

using namespace algorithm;

activationWaiter::activationWaiter(stAlgorithm& aAlgorithm)
	:algorithm(aAlgorithm) {}

bool activationWaiter::operator==(const activationWaiter& other) const {
	return activationWaitCounter == other.activationWaitCounter;
}

void activationWaiter::onNewTrend() {
	if (algorithm.getState() != getIntState(eCustomState::ACTIVATION_WAIT)) {
		return;
	}
	if (algorithm.getData().getActivationWaiterResetAllowed()) {
		algorithm.setState(getIntState(eBaseState::NONE));
		activationWaitCounter = 0;
	}
	else {
		activationWaitCounter = algorithm.getData().getActivationWaiterRange();
	}
}

void activationWaiter::start() {
	if (const auto range = algorithm.getData().getActivationWaiterRange(); range >= 0) {
		algorithm.setState(getIntState(eCustomState::ACTIVATION_WAIT));
		activationWaitCounter = algorithm.getData().getActivationWaiterRange();
	}
	else {
		utils::logError("activationWaiter::start wrong range");
	}
}

bool activationWaiter::check() {
	if (algorithm.getState() != getIntState(eCustomState::ACTIVATION_WAIT)) {
		return false;
	}
	const auto trendActivation = algorithm.getActualSuperTrend();
	if (activationWaitCounter == 0) {
		const auto isTrendUp = algorithm.getIsTrendUp();
		const auto& open = algorithm.getCandle().open;
		if (isTrendUp && utils::isGreater(open, trendActivation)) {
			algorithm.openOrder(eOrderState::LONG, open);
			return true;
		}
		else if (!isTrendUp && utils::isLess(open, trendActivation)) {
			algorithm.openOrder(eOrderState::SHORT, open);
			return true;
		}
		activationWaitCounter = algorithm.getData().getActivationWaiterRange();
	}
	else {
		const auto& candle = algorithm.getCandle();
		const auto fullCandleCheck = algorithm.getData().getActivationWaiterFullCandleCheck();
		auto minimum = (fullCandleCheck) ? candle.low : utils::minFloat(candle.open, candle.close);
		auto maximum = (fullCandleCheck) ? candle.high : utils::maxFloat(candle.open, candle.close);
		const auto isTrendUp = algorithm.getIsTrendUp();
		if ((isTrendUp && utils::isGreater(minimum, trendActivation)) || (!isTrendUp && utils::isLess(maximum, trendActivation))) {
			activationWaitCounter -= 1;
		}
		else {
			activationWaitCounter = algorithm.getData().getActivationWaiterRange();
		}
	}
	if (algorithm.isNewTrendChanged()) {
		activationWaitCounter = algorithm.getData().getActivationWaiterRange();
	}
	return false;
}