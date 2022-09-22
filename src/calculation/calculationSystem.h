#pragma once
#include "combinationFactory.hpp"
#include "market/indicatorsSystem.h"
#include "market/marketRules.h"
#include "outputHelper.h"
#include <mutex>

namespace calculation {
	class calculationSystem {
	public:
		calculationSystem();
		void calculate();
	private:
		void loadSettings();
		void printProgress(int index, int summary);
		void saveFinalData(const std::string& aTicker, market::eCandleInterval aInterval);
		combinationsJsons balanceResultsByMaxLoss();
		void uniteResults(const combinationsCalculations& infos, const combinationsJsons& jsons);

		template<typename algorithmType>
		void calculateInternal() {
			//processCalculations<algorithmType>();
			auto balancedJsons = balanceResultsByMaxLoss();
			auto balancedInfos = recalculateBalancedData<algorithmType>(balancedJsons);
			uniteResults(balancedInfos, balancedJsons);
		}

		template<typename algorithmType, typename algorithmDataType>
		void iterate(combinationFactory<algorithmDataType>& aFactory, market::eCandleInterval aTimeframe, int aThread) {
			auto& threadResults = threadsData[aThread];
			const auto& threadData = aFactory.getThreadData(aThread);
			for (const auto& data : threadData) {
				auto algorithm = algorithmType(data, aTimeframe);
				const auto result = algorithm.calculate(candlesSource);
				if (result) {
					threadResults.push_back(algorithm.getJsonData());
				}
				aFactory.incrementThreadIndex(aThread);
				printProgress(aFactory.getCurrentIndex(), combinations);
			}
		}

		template<typename algorithmType>
		void processCalculations() {
			for (const auto& [ticker, timeframe] : calculations) {
				auto json = utils::readFromJson("assets/candles/" + ticker + '_' + getCandleIntervalApiStr(timeframe));
				candlesSource = utils::parseCandles(json);
				threadsData.resize(threadsAmount);

				auto factory = combinationFactory<algorithmType::algorithmDataType>(threadsAmount, ticker);
				combinations = factory.getCombinationsAmount();
				std::vector<std::future<void>> futures;
				for (size_t i = 0; i < threadsAmount; ++i) {
					futures.push_back(std::async(std::launch::async, [this, &factory, timeframe, i]() { return iterate<algorithmType>(factory, timeframe, static_cast<int>(i)); }));
				}
				for (auto& future : futures) {
					future.wait();
				}
				factory.onFinish();
				saveFinalData(ticker, timeframe);
				utils::log("calculationSystem::calculate finish - " + ticker + '\n');
				progress = 0;
			}
			candlesSource.clear();
		}

		template<typename algorithmType>
		combinationsCalculations recalculateBalancedData(const combinationsJsons& balancedData) {
			combinationsCalculations unitedInfo(balancedData.size());
			int index = 0;
			const int summary = calculations.size() * balancedData.size();
			for (const auto& [ticker, timeframe] : calculations) {
				auto json = utils::readFromJson("assets/candles/" + ticker + '_' + getCandleIntervalApiStr(timeframe));
				auto candles = utils::parseCandles(json);
				for (const auto& [id, jsonData] : balancedData) {
					auto algData = algorithmType::algorithmDataType{ ticker };
					if (!algData.initFromJson(jsonData)) {
						utils::logError("\ncalculationSystem::recalculateBalancedData wrong alrorithm json data");
						continue;
					}

					auto algorithm = algorithmType(algData, timeframe);
					const auto result = algorithm.calculate(candles);
					const auto jsonData = algorithm.getJsonData();
					if (result) {
						unitedInfo[id].push_back(getCalculationInfo(ticker, jsonData));
					}
					else {
						utils::logError("\ncalculationSystem::recalculateBalancedData wrong balance - " + ticker + " - " + jsonData);
					}
					printProgress(++index, summary);
				}
			}
			utils::log("\ncalculation::recalculateBalancedData finished");
			return unitedInfo;
		}

		std::vector<std::vector<Json>> threadsData;
		std::vector<market::candle> candlesSource;
		std::mutex printMutex;
		
		std::string algorithmType;
		calculationsType calculations;
		size_t threadsAmount = 0;
		int parabolaDegree = 0;

		int progress = 0;
		size_t combinations = 0;
	};
}
