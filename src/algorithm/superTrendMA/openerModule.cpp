#include "openerModule.h"
#include "stMAlgorithm.h"

using namespace algorithm;

openerModule::openerModule(stMAlgorithm& aAlgorithm)
	:algorithm(aAlgorithm) {}

bool openerModule::check() {
	if (isPrevPositionCorrect()) {
		return algorithm.openOrder((algorithm.getIndicators().isSuperTrendUp()) ? eOrderState::LONG : eOrderState::SHORT, algorithm.getCandle().open);
	}

	return false;
}

bool openerModule::isPrevPositionCorrect() const {
	const auto sameCandleAsLastClose = algorithm.getCandle().time == lastClosedOrder.first;

	if (algorithm.getIndicators().isSuperTrendUp()) {
		return !sameCandleAsLastClose || (sameCandleAsLastClose && lastClosedOrder.second == eOrderState::SHORT);
	}
	else {
		return !sameCandleAsLastClose || (sameCandleAsLastClose && lastClosedOrder.second == eOrderState::LONG);
	}
}

void openerModule::onOpenOrder() {
}

void openerModule::onCloseOrder(eOrderState state, double profit) {
	lastClosedOrder.first = algorithm.getCandle().time;
	lastClosedOrder.second = state;
}