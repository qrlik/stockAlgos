#pragma once
#include "candle.h"
#include <vector>
#include <deque>

namespace market {
	class indicatorsData;
	class indicatorsSystem {
	public:
		indicatorsSystem(const indicatorsData& aData);
		bool operator==(const indicatorsSystem& aOther) const;
		void initFromJson(const Json& aValue);

		void processCandle(candle& aCandle);
		bool isInited() const { return inited; }
		std::pair<double, bool> getSuperTrend() const { return { superTrend, trendIsUp }; }
	private:
		bool calculateMA(candle& aCandle);
		double calculateTrueRangeWMA() const;
		double calculateTrueRangeEMA(double aAlpha);
		double calculateTrueRangeMA();
		bool calculateRangeAtr(candle& aCandle);
		void calculateSuperTrend(candle& aCandle);
		bool checkSkip();

		const indicatorsData& data;

		std::deque<double> trList;
		std::deque<double> closeList;
		candle prevCandle;

		double atr = 0.f;

		double lastUpperBand = 0.0;
		double lastLowerBand = 0.0;
		double superTrend = 0.f;
		bool trendIsUp = false;

		double firstMASum = 0.0;
		double secondMASum = 0.0;
		double firstMA = 0.0;
		double secondMA = 0.0;

		int candlesCounter = 0;
		bool inited = false;
	};
}
