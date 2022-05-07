#pragma once
#include "candle.h"
#include "indicatorsData.h"
#include <vector>
#include <deque>

namespace market {
	class indicatorsSystem {
	public:
		indicatorsSystem(const indicatorsData& aData);
		void getProcessedCandles(std::vector<candle>& aCandles, int aAmount = 0);

	private:
		double calculateTrueRangeWMA() const;
		double calculateTrueRangeEMA(double aAlpha);
		double calculateTrueRangeMA();
		void calculateSuperTrend(candle& aCandle);
		void calculateRangeAtr(candle& aCandle);
		void processCandle(candle& aCandle);

		const indicatorsData& data;

		std::deque<double> trList;
		candle prevCandle;

		double lastUpperBand = 0.0;
		double lastLowerBand = 0.0;
		double lastTrend = 0.0;
		double lastClose = 0.0;
		int lastCandleTime = 0;
	};
}
