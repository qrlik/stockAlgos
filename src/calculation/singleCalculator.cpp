#include "singleCalculator.h"
#include "../algorithm/moneyMaker.h"
#include "../structs/algorithmData.h"
#include "../indicators/indicators.h"
#include "../utils/utils.h"

using namespace calculation;

void calculation::singleCalculation() {
	algorithmData data{};
    data.atrType = indicators::eAtrType::EMA;
    data.atrSize = 15;
    data.stFactor = 3.0;

    data.dealPercent = 5.0;
    data.leverage = 50;

    data.activationPercent = 0.0;
    data.stopLossPercent = 1.0;
    data.minimumProfitPercent = 0.5;

    data.dynamicSLPercent = -1.0;
    data.dynamicSLTrendMode = true;

    data.touchOpenerActivationWaitMode = true;

    data.breakOpenerEnabled = true;
    data.breakOpenerActivationWaitMode = true;
    data.alwaysUseNewTrend = true;

    data.activationWaiterResetAllowed = true;
    data.activationWaiterRange = 0;
    data.activationWaiterFullCandleCheck = true;

    data.stopLossWaiterEnabled = true;
    data.stopLossWaiterResetAllowed = true;
    data.stopLossWaiterRange = 0;
    data.stopLossWaiterFullCandleCheck = true;

    auto json = utils::readFromJson("assets/candles/1h_year");
    auto candles = utils::parseCandles(json);
    indicators::getProcessedCandles(candles, data.atrType, data.atrSize, data.stFactor, 8760);

    auto mm = algorithm::moneyMaker(data, 100.0);
    mm.calculate(candles);
    //auto fullCash = mm.getFullCash();
    //auto breakvar = 5;
}