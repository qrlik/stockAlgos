#pragma once
#include "combinationFactory.hpp"
#include "market/indicatorsSystem.h"
#include "market/marketRules.h"
#include "outputHelper.h"
#include <future>
#include <mutex>

namespace calculation {
	class calculationSystem {
	public:
		calculationSystem();
		void calculate();
	private:
		void loadSettings();
		bool saveFinalData(const std::string& aTicker, market::eCandleInterval aInterval);
		combinationsJsons balanceResultsByMaxLoss(size_t threadsAmount);
		void uniteResults(Json balancedData);

		template<typename algorithmType>
		void calculateInternal() {
			auto balancedData = utils::readFromJson(utils::balancedDataDir);
			if (balancedData.is_null()) {
				if (!processCalculations<algorithmType>()) {
					return;
				}
				auto balancedAlgos = balanceResultsByMaxLoss(threadsAmount);
				balancedData = recalculateBalancedData<algorithmType>(std::move(balancedAlgos));
			}
			uniteResults(std::move(balancedData));
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
				utils::printProgress(static_cast<int>(aFactory.getCurrentIndex()), static_cast<int>(combinations));
			}
		}

		template<typename algorithmType>
		bool processCalculations() {
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
				if (!saveFinalData(ticker, timeframe)) {
					return false;
				}
				utils::log("calculationSystem::calculate finish - " + ticker + '\n');
				utils::resetProgress();
			}
			candlesSource.clear();
			return true;
		}

		template<typename algorithmType>
		Json recalculateBalancedData(combinationsJsons balancedAlgos) {
			Json balancedData;
			for (auto& [id, algoData] : balancedAlgos) {
				balancedData[std::to_string(id)]["data"] = std::move(algoData);
			}

			int index = 0;
			const int summary = static_cast<int>(calculations.size() * balancedAlgos.size());
			for (const auto& [ticker, timeframe] : calculations) {
				auto json = utils::readFromJson("assets/candles/" + ticker + '_' + getCandleIntervalApiStr(timeframe));
				auto candles = utils::parseCandles(json);
				for (const auto& [id, idData] : balancedData.items()) {
					auto algData = algorithmType::algorithmDataType{ ticker };
					if (!algData.initFromJson(idData["data"])) {
						utils::logError("\ncalculationSystem::recalculateBalancedData wrong alrorithm json data");
						continue;
					}

					auto algorithm = algorithmType(algData, timeframe);
					const auto result = algorithm.calculate(candles);
					if (result) {
						auto algoJson = algorithm.getJsonData();
						algoJson.erase("data");
						idData["tickers"][ticker] = std::move(algoJson);
					}
					else {
						utils::logError("\ncalculationSystem::recalculateBalancedData wrong balance - " + ticker + " - " + id);
					}
					utils::printProgress(++index, summary);
				}
			}
			utils::saveToJson(utils::balancedDataDir, balancedData);
			utils::resetProgress();
			utils::log("\ncalculation::recalculateBalancedData finished");
			return balancedData;
		}

		std::vector<std::vector<Json>> threadsData;
		std::vector<market::candle> candlesSource;
		
		std::string algorithmType;
		calculationsType calculations;
		size_t threadsAmount = 0;
		int parabolaDegree = 0;

		size_t combinations = 0;
	};
}
