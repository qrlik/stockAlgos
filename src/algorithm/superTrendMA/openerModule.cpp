#include "openerModule.h"
#include "stMAlgorithm.h"

using namespace algorithm;

openerModule::openerModule(stMAlgorithm& aAlgorithm)
	:algorithm(aAlgorithm) {}

bool openerModule::check() {
	auto touchedThisCandle = checkTrendTouch();
	return tryToOpenOrder(touchedThisCandle);
}

bool openerModule::checkTrendTouch() {
	if (touchActivated) {
		return false;
	}
	const auto trendActivation = getActivationPrice();
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

double openerModule::getActivationPrice() const {
	const auto superTrend = algorithm.getIndicators().getSuperTrend();
	const auto sign = (algorithm.getIndicators().isSuperTrendUp()) ? 1 : -1;
	return superTrend * (100 + sign * algorithm.getData().getActivationPercent()) / 100.0;
}

double openerModule::getDeactivationPrice() const {
	const auto percent = algorithm.getData().getDeactivationPercent();
	if (utils::isLessOrEqual(percent, 0.0)) {
		return -1.0;
	}
	const auto superTrend = algorithm.getIndicators().getSuperTrend();
	const auto sign = (algorithm.getIndicators().isSuperTrendUp()) ? 1 : -1;
	const auto fullPercent = percent + algorithm.getData().getActivationPercent();
	return superTrend * (100 + sign * fullPercent) / 100.0;
}

double openerModule::getOpenPrice(bool aIsTochedThisCandle) const {
	if (!aIsTochedThisCandle) {
		return algorithm.getCandle().open;
	}
	const auto activationPrice = getActivationPrice();
	if (algorithm.getIndicators().isSuperTrendUp()) {
		return utils::minFloat(activationPrice, algorithm.getCandle().open);
	}
	else {
		return utils::maxFloat(activationPrice, algorithm.getCandle().open);
	}
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
	const auto openPrice = getOpenPrice(aIsTochedThisCandle);
	const auto deactivationPrice = getDeactivationPrice();

	if (algorithm.getIndicators().isSuperTrendUp()) {
		if (isFirstMAGrowing && isSecondMAGrowing) {
			if (utils::isGreater(secondMA, firstMA)) {
				if (!sameCandleAsLastClose || (sameCandleAsLastClose && lastClosedOrder.second == eOrderState::SHORT)) {
					if (algorithm.getCloserModule().isNeedToClose(true)) {
						touchActivated = false;
						return !aIsTochedThisCandle;
					}
					else if (utils::isGreater(deactivationPrice, 0.0) && utils::isGreaterOrEqual(openPrice, deactivationPrice)) {
						touchActivated = false;
						return false;
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
					else if (utils::isGreater(deactivationPrice, 0.0) && utils::isLessOrEqual(openPrice, deactivationPrice)) {
						touchActivated = false;
						return false;
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