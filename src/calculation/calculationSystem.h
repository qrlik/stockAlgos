#pragma once
#include "combinationFactory.hpp"
#include "market/indicatorsSystem.h"
#include "market/marketRules.h"
#include <mutex>

namespace calculation {
	class calculationSystem {
	public:
		calculationSystem();
		void calculate();
	private:
		void loadSettings();
		void printProgress(size_t aIndex);
		void saveFinalData(const std::string& aTicker, market::eCandleInterval aInterval);

		template<typename algorithmType, typename algorithmDataType>
		void iterate(combinationFactory<algorithmDataType>& aFactory, int aThread) {
			std::vector<market::candle> candles;
			auto& threadResults = threadsData[aThread];
			const auto& threadData = aFactory.getThreadData(aThread);
			for (const auto& data : threadData) {
				auto algorithm = algorithmType(data);
				const auto result = algorithm.calculate(candlesSource);
				if (result) {
					threadResults.push_back(algorithm.getJsonData());
				}
				aFactory.incrementThreadIndex(aThread);
				printProgress(aFactory.getCurrentIndex());
			}
		}

		template<typename algorithmType>
		void calculateInternal() {
			for (const auto& [ticker, timeframe] : calculations) {
				if (!MARKET_DATA->loadTickerData(ticker)) {
					utils::logError("calculationSystem::calculate wrong ticker - " + ticker);
					continue;
				}
				auto json = utils::readFromJson("assets/candles/" + ticker + '/' + getCandleIntervalApiStr(timeframe));
				candlesSource = utils::parseCandles(json);
				threadsData.resize(threadsAmount);

				std::vector<std::future<void>> futures;
				auto factory = combinationFactory<algorithmType::algorithmDataType>(threadsAmount);
				combinations = factory.getCombinationsAmount();
				for (size_t i = 0; i < threadsAmount; ++i) {
					futures.push_back(std::async(std::launch::async, [this, &factory, i]() { return iterate<algorithmType>(factory, static_cast<int>(i)); }));
				}
				for (auto& future : futures) {
					future.wait();
				}
				factory.onFinish();
				saveFinalData(ticker, timeframe);
				utils::log("calculationSystem::calculate finish - " + ticker);
			}
		}

		std::vector<std::vector<Json>> threadsData;
		std::vector<market::candle> candlesSource;
		std::mutex printMutex;
		
		std::string algorithmType;
		std::vector<std::pair<std::string, market::eCandleInterval>> calculations;
		size_t threadsAmount = 0;
		double weightPrecision = 0.0;
		int parabolaDegree = 0;

		double progress = 0.0;
		size_t combinations = 0;
	};
}
