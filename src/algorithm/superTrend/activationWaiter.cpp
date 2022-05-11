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
	if (algorithm.getState() != eState::ACTIVATION_WAIT) {
		return;
	}
	if (algorithm.getData().getActivationWaiterResetAllowed()) {
		algorithm.setState(eState::NONE);
		activationWaitCounter = 0;
	}
	else {
		activationWaitCounter = algorithm.getData().getActivationWaiterRange();
	}
}

void activationWaiter::start() {
	if (const auto range = algorithm.getData().getActivationWaiterRange(); range >= 0) {
		algorithm.setState(eState::ACTIVATION_WAIT);
		activationWaitCounter = algorithm.getData().getActivationWaiterRange();
	}
	else {
		utils::logError("activationWaiter::start wrong range");
	}
}

bool activationWaiter::check() {
	if (algorithm.getState() != eState::ACTIVATION_WAIT) {
		return false;
	}
	const auto trendActivation = algorithm.getActualSuperTrend();
	if (activationWaitCounter == 0) {
		const auto isTrendUp = algorithm.getIsTrendUp();
		const auto& open = algorithm.getCandle().open;
		if (isTrendUp && open > trendActivation) {
			algorithm.openOrder(eOrderState::LONG, open);
			return true;
		}
		else if (!isTrendUp && open < trendActivation) {
			algorithm.openOrder(eOrderState::SHORT, open);
			return true;
		}
		activationWaitCounter = algorithm.getData().getActivationWaiterRange();
	}
	else {
		const auto& candle = algorithm.getCandle();
		const auto fullCandleCheck = algorithm.getData().getActivationWaiterFullCandleCheck();
		auto minimum = (fullCandleCheck) ? candle.low : std::min(candle.open, candle.close);
		auto maximum = (fullCandleCheck) ? candle.high : std::max(candle.open, candle.close);
		const auto isTrendUp = algorithm.getIsTrendUp();
		if ((isTrendUp && minimum > trendActivation) || (!isTrendUp && maximum < trendActivation)) {
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