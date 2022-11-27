#include "stMAlgorithm.h"

using namespace algorithm;

stMAlgorithm::stMAlgorithm(const stMAlgorithmData& aData, market::eCandleInterval aTimeframe) :
	baseClass(aData, aTimeframe),
	opener(*this),
	closer(*this) {}

bool stMAlgorithm::operator==(const stMAlgorithm& aOther) const {
	auto result = baseClass::operator==(aOther);
	return result;
}

void stMAlgorithm::preLoop() {}

bool stMAlgorithm::loop() {
	const auto curState = getState();
	if (curState == getIntState(eBaseState::NONE)) {
		return opener.check();
	}
	else if (curState == getIntState(eBaseState::LONG) || curState == getIntState(eBaseState::SHORT)) {
		return closer.check();
	}
	return false;
}

void stMAlgorithm::onOpenOrder() {
	opener.onOpenOrder();
}

void stMAlgorithm::onCloseOrder(eOrderState aState, double aProfit) {
	opener.onCloseOrder(aState, aProfit);
}

void stMAlgorithm::updateOrderStopLoss(double aStopLoss) {
	getOrder().updateStopLoss(aStopLoss);
}
