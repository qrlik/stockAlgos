#pragma once
#include "candle.h"
#include "indicatorsData.h"
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
		double getSuperTrend() const { return superTrend; }
		bool isSuperTrendUp() const { return trendIsUp; }
		double getFirstMA() const { return firstMA; }
		double getSecondMA() const { return secondMA; }
		double getRSI() const { return rsi; }
	private:
		double calculateCustomMA(market::eAtrType aType, const std::deque<double>& aList, double lastValue) const;
		bool calculateMA(candle& aCandle);
		bool calculateRSI(candle& aCandle);
		bool calculateRangeAtr(candle& aCandle);
		void calculateSuperTrend(candle& aCandle);
		bool checkSkip();

		const indicatorsData& data;

		std::deque<double> trList; // atr
		double atr = 0.f;

		double lastUpperBand = 0.0; // supertrend
		double lastLowerBand = 0.0;
		double superTrend = 0.f;
		bool trendIsUp = false;

		std::deque<double> closeList; // ma
		double firstMASum = 0.0;
		double secondMASum = 0.0;
		double firstMA = 0.0;
		double secondMA = 0.0;

		std::deque<double> upList; // rsi
		std::deque<double> downList;
		double upMa = 0.0;
		double downMa = 0.0;
		double rsi = 0.0;

		candle prevCandle;

		int candlesCounter = 0;
		bool inited = false;
	};
}
