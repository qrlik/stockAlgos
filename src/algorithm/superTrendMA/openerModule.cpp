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
	const auto activationPrice = superTrend * (100 + sign * algorithm.getData().getActivationPercent()) / 100.0;
	return utils::round(activationPrice, algorithm.getData().getMarketData().getPricePrecision(algorithm.getCandle().high));
}

double openerModule::getDeactivationPrice() const {
	const auto percent = algorithm.getData().getDeactivationPercent();
	if (utils::isLessOrEqual(percent, 0.0)) {
		return -1.0;
	}
	const auto superTrend = algorithm.getIndicators().getSuperTrend();
	const auto sign = (algorithm.getIndicators().isSuperTrendUp()) ? 1 : -1;
	const auto fullPercent = percent + algorithm.getData().getActivationPercent();

	const auto deactivationPrice = superTrend * (100 + sign * fullPercent) / 100.0;
	return utils::round(deactivationPrice, algorithm.getData().getMarketData().getPricePrecision());
}

double openerModule::getOpenPrice(bool aIsTochedThisCandle) const {
	// to do
	// 
	// add activation percent == -1

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

bool openerModule::isMADirectionCorrect() const {
	const auto isFirstMAGrowing = algorithm.getMAModule().isFirstUp();
	const auto isSecondMAGrowing = algorithm.getMAModule().isSecondUp();

	if (algorithm.getIndicators().isSuperTrendUp()) {
		return isFirstMAGrowing && isSecondMAGrowing;
	}
	else {
		return !isFirstMAGrowing && !isSecondMAGrowing;
	}
}

bool openerModule::isMAPositionCorrect() const {
	const auto firstMA = algorithm.getIndicators().getFirstMA();
	const auto secondMA = algorithm.getIndicators().getSecondMA();

	if (algorithm.getIndicators().isSuperTrendUp()) {
		return utils::isGreater(secondMA, firstMA);
	}
	else {
		return utils::isLess(secondMA, firstMA);
	}
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

bool openerModule::isDeactivationPriceCross(double openPrice) const {
	const auto deactivationPrice = getDeactivationPrice();
	if (utils::isLessOrEqual(deactivationPrice, 0.0)) {
		return false;
	}

	if (algorithm.getIndicators().isSuperTrendUp()) {
		return utils::isGreaterOrEqual(openPrice, deactivationPrice);
	}
	else {
		return utils::isLessOrEqual(openPrice, deactivationPrice);
	}
}

bool openerModule::isCloseAfterOpen() const {
	return algorithm.getCloserModule().isNeedToClose(algorithm.getIndicators().isSuperTrendUp());
}

bool openerModule::tryToOpenOrder(bool aIsTochedThisCandle) {
	if (!touchActivated) {
		return false;
	}

	const auto openPrice = getOpenPrice(aIsTochedThisCandle);
	if (isMADirectionCorrect() && isMAPositionCorrect() && isPrevPositionCorrect()) {
		if (isDeactivationPriceCross(openPrice)) {
			touchActivated = false; // check without
			return false;
		}
		else if (isCloseAfterOpen()) {
			touchActivated = false; // check without
			return false;
		}
		else {
			return algorithm.openOrder((algorithm.getIndicators().isSuperTrendUp()) ? eOrderState::LONG : eOrderState::SHORT, openPrice);
		}
	}

	return false;
}

void openerModule::onOpenOrder() {
	touchActivated = false;
}

void openerModule::onCloseOrder(eOrderState aState, double aProfit) {
	lastClosedOrder.first = algorithm.getCandle().time;
	lastClosedOrder.second = aState;
}