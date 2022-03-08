#include "singleCalculator.h"
#include "../algorithm/moneyMaker.h"
#include "../structs/algorithmData.h"
#include "../indicators/indicators.h"
#include "../utils/utils.h"
#include <iostream>

using namespace calculation;

void calculation::singleCalculation() {
	algorithmData data{};
    data.atrType = indicators::eAtrType::SMA;
    data.atrSize = 15;
    data.stFactor = 3.0;

    data.dealPercent = 5.0;
    data.leverage = 50;

    data.activationPercent = 2.0;
    data.stopLossPercent = 2.0;
    data.minimumProfitPercent = 0.5;

    data.dynamicSLPercent = 0.25;
    data.dynamicSLTrendMode = false;

    data.touchOpenerActivationWaitMode = false;

    data.breakOpenerEnabled = true;
    data.breakOpenerActivationWaitMode = false;
    data.alwaysUseNewTrend = false;

    data.activationWaiterResetAllowed = false;
    data.activationWaiterRange = -1;
    data.activationWaiterFullCandleCheck = false;

    data.stopLossWaiterEnabled = true;
    data.stopLossWaiterResetAllowed = false;
    data.stopLossWaiterRange = 0;
    data.stopLossWaiterFullCandleCheck = false;

    auto json = utils::readFromJson("assets/candles/1h_year");
    auto candles = utils::parseCandles(json);
    indicators::getProcessedCandles(candles, data.atrType, data.atrSize, data.stFactor, 8760);

    auto mm = algorithm::moneyMaker(data, 100.0);
    mm.setWithLogs(true);
    mm.calculate(candles);
    std::cout << mm.getFullCash() << '\n';
}