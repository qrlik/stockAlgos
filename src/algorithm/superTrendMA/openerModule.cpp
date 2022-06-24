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
					if (algorithm.getCloserModule().isNeedToClose(true)) {
						touchActivated = false;
						return !aIsTochedThisCandle;
					}
					else {
						algorithm.openOrder(eOrderState::LONG, openPrice);
						return true;
					}
				}
			}
		}
	}
	else {
		if (!isFirstMAGrowing && !isSecondMAGrowing) {
			if (utils::isGreater(firstMA, secondMA)) {
				if (!sameCandleAsLastClose || (sameCandleAsLastClose && lastClosedOrder.second == eOrderState::LONG)) {
					if (algorithm.getCloserModule().isNeedToClose(false)) {
						touchActivated = false;
						return !aIsTochedThisCandle;
					}
					else {
						algorithm.openOrder(eOrderState::SHORT, openPrice);
						return true;
					}
				}
			}
		}
	}
	return false;
}

void openerModule::onOpenOrder() {
	touchActivated = false;
	lastOpenRsi = algorithm.getIndicators().getRSI();
}

void openerModule::onCloseOrder(eOrderState aState, double aProfit) {
	lastClosedOrder.first = algorithm.getCandle().time;
	lastClosedOrder.second = aState;

	auto flooredRsi = static_cast<int>(utils::ceil(lastOpenRsi, 10.0));
	if (aState == eOrderState::LONG) {
		if (utils::isGreater(aProfit, 0.0)) {
			++rsiProfitLongs[flooredRsi];
		}
		else {
			++rsiUnprofitLongs[flooredRsi];
		}
	}
	else {
		if (utils::isGreater(aProfit, 0.0)) {
			++rsiProfitShorts[flooredRsi];
		}
		else {
			++rsiUnprofitShorts[flooredRsi];
		}
	}
}