#include "singleCalculator.h"
#include "../algorithm/moneyMaker.h"
#include "../structs/algorithmData.h"
#include "../indicators/indicators.h"
#include "../utils/utils.h"

using namespace calculation;

void calculation::singleCalculation() {
	algorithmData data{};
    data.atrType = indicators::eAtrType::RMA;
    data.atrSize = 14;
    data.stFactor = 5;

    data.dealPercent = 5;
    data.leverage = 10;

    data.activationPercent = 0.25;
    data.stopLossPercent = 1.25;
    data.minimumProfitPercent = 0.25;

    data.dynamicSLPercent = 0.25;

    data.touchOpenerActivationWaitMode = false;

    data.breakOpenerEnabled = true;
    data.breakOpenerActivationWaitMode = false;
    data.alwaysUseNewTrend = true;

    data.stopLossWaiterEnabled = true;
    data.stopLossWaiterResetAllowed = false;
    data.stopLossWaiterRange = 1;
    data.stopLossWaiterFullCandleCheck = true;

    auto json = utils::readFromJson("assets/candles/1h_year");
    auto candles = utils::parseCandles(json);
    indicators::getProcessedCandles(candles, data.atrType, data.atrSize, data.stFactor, 8760);

    auto mm = algorithm::moneyMaker(data, 100.0);
    mm.calculate(candles);
    //auto breakvar = 5;
}