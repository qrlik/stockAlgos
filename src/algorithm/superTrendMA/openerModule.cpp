#include "openerModule.h"
#include "stMAlgorithm.h"

using namespace algorithm;

openerModule::openerModule(stMAlgorithm& aAlgorithm)
	:algorithm(aAlgorithm) {}

bool openerModule::check() {
	return tryToOpenOrder();
}

double openerModule::getOpenOffsetPrice() const {
	const auto superTrend = algorithm.getIndicators().getSuperTrend();
	const auto sign = (algorithm.getIndicators().isSuperTrendUp()) ? 1 : -1;
	const auto openOffsetPrice = superTrend * (100 + sign * algorithm.getData().getOpenOffsetPercent()) / 100.0;
	return utils::round(openOffsetPrice, algorithm.getData().getMarketData().getPricePrecision());
}

double openerModule::getOpenPrice() const {
	const auto openOffsetPrice = getOpenOffsetPrice();
	const auto& candle = algorithm.getCandle();
	if (algorithm.getIndicators().isSuperTrendUp()) {
		if (utils::isGreater(candle.low, openOffsetPrice)) {
			return -1.0;
		}
		else if (utils::isLessOrEqual(candle.open, openOffsetPrice)) {
			return candle.open;
		}
		else {
			return openOffsetPrice;
		}
	}
	else {
		if (utils::isLess(candle.high, openOffsetPrice)) {
			return -1.0;
		}
		else if (utils::isGreaterOrEqual(candle.open, openOffsetPrice)) {
			return candle.open;
		}
		else {
			return openOffsetPrice;
		}
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

bool openerModule::isCloseAfterOpen() const {
	return algorithm.getCloserModule().isNeedToClose(algorithm.getIndicators().isSuperTrendUp());
}

bool openerModule::tryToOpenOrder() {
	if (isMADirectionCorrect() && isMAPositionCorrect() && isPrevPositionCorrect()) {
		if (isCloseAfterOpen()) {
			return false;
		}
		else if (auto openPrice = getOpenPrice(); utils::isGreater(openPrice, 0.0)) {
			algorithm.openOrder((algorithm.getIndicators().isSuperTrendUp()) ? eOrderState::LONG : eOrderState::SHORT, openPrice);
			return true;
		}
	}

	return false;
}

void openerModule::onOpenOrder() {
}

void openerModule::onCloseOrder(eOrderState aState, double aProfit) {
	lastClosedOrder.first = algorithm.getCandle().time;
	lastClosedOrder.second = aState;
}