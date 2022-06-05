#include "openerModule.h"
#include "stMAlgorithm.h"

using namespace algorithm;

openerModule::openerModule(stMAlgorithm& aAlgorithm)
	:algorithm(aAlgorithm) {}

bool openerModule::check() {
	auto touchedThisCandle = checkTrendTouch();
	//if (checkTrendTouch()) { // try to allow the same candle
	//	return false;
	//}
	return tryToOpenOrder(touchedThisCandle);
}

bool openerModule::checkTrendTouch() {
	if (touchActivated) {
		return false;
	}
	const auto trendActivation = algorithm.getIndicators().getSuperTrend();
	const auto isTrendUp = algorithm.getIndicators().isSuperTrendUp();
	const auto& candle = algorithm.getCandle();
	if (isTrendUp && utils::isLessOrEqual(candle.low, trendActivation)) {
		touchActivated = true;
	}
	else if (!isTrendUp && utils::isGreaterOrEqual(candle.high, trendActivation)) {
		touchActivated = true;
	}
	return touchActivated;
}

bool openerModule::tryToOpenOrder(bool aIsTochedThisCandle) {
	if (!touchActivated) {
		return false;
	}
	const auto sameCandleAsLastClose = algorithm.getCandle().time == lastClosedOrder.first;
	const auto isFirstMAGrowing = algorithm.getMAModule().isFirstUp();
	const auto isSecondMAGrowing = algorithm.getMAModule().isSecondUp();
	const auto firstMA = algorithm.getIndicators().getFirstMA();
	const auto secondMA = algorithm.getIndicators().getSecondMA();
	const auto openPrice = aIsTochedThisCandle ? algorithm.getIndicators().getSuperTrend() : algorithm.getCandle().open;
	if (algorithm.getIndicators().isSuperTrendUp()) {
		if (isFirstMAGrowing && isSecondMAGrowing) {
			if (utils::isGreater(secondMA, firstMA)) {
				if (!sameCandleAsLastClose || (sameCandleAsLastClose && lastClosedOrder.second == eOrderState::SHORT)) {
					algorithm.openOrder(eOrderState::LONG, openPrice);
					return true;
				}
			}
		}
	}
	else {
		if (!isFirstMAGrowing && !isSecondMAGrowing) {
			if (utils::isGreater(firstMA, secondMA)) {
				if (!sameCandleAsLastClose || (sameCandleAsLastClose && lastClosedOrder.second == eOrderState::LONG)) {
					algorithm.openOrder(eOrderState::SHORT, openPrice);
					return true;
				}
			}
		}
	}
	return false;
}