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
		void uniteResults();

		template<typename algorithmType, typename algorithmDataType>
		void iterate(combinationFactory<algorithmDataType>& aFactory, market::eCandleInterval aTimeframe, int aThread) {
			std::vector<market::candle> candles;
			auto& threadResults = threadsData[aThread];
			const auto& threadData = aFactory.getThreadData(aThread);
			for (const auto& data : threadData) {
				auto algorithm = algorithmType(data, aTimeframe);
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
			//processCalculations<algorithmType>();
			uniteResults();
		}

		template<typename algorithmType>
		void processCalculations() {
			auto factory = combinationFactory<algorithmType::algorithmDataType>(threadsAmount);
			combinations = factory.getCombinationsAmount();
			for (const auto& [ticker, timeframe] : calculations) {
				if (!MARKET_DATA->loadTickerData(ticker)) {
					utils::logError("calculationSystem::calculate wrong ticker - " + ticker);
					continue;
				}
				progress = 0;
				auto json = utils::readFromJson("assets/candles/" + ticker + '/' + getCandleIntervalApiStr(timeframe));
				candlesSource = utils::parseCandles(json);
				threadsData.resize(threadsAmount);

				std::vector<std::future<void>> futures;
				for (size_t i = 0; i < threadsAmount; ++i) {
					futures.push_back(std::async(std::launch::async, [this, &factory, timeframe, i]() { return iterate<algorithmType>(factory, timeframe, static_cast<int>(i)); }));
				}
				for (auto& future : futures) {
					future.wait();
				}
				factory.onFinish();
				saveFinalData(ticker, timeframe);
				utils::log("calculationSystem::calculate finish - " + ticker);
			}
		}

		struct calculationInfo {
			double weight = 0.0;
			double cash = 0.0;
			double profitsFactor = 0.0;
			double recoveryFactor = 0.0;
			int ordersAmount = 0;
		};

		std::vector<std::vector<Json>> threadsData;
		std::vector<market::candle> candlesSource;
		std::mutex printMutex;
		
		std::string algorithmType;
		std::vector<std::pair<std::string, market::eCandleInterval>> calculations;
		size_t threadsAmount = 0;
		double weightPrecision = 0.0;
		int parabolaDegree = 0;

		int progress = 0;
		size_t combinations = 0;
	};
}
