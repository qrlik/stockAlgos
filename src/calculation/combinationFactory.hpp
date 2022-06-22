#pragma once
#include "json/json.hpp"
#include <algorithm>
#include <random>

namespace {
	template<typename T>
	std::vector<T> iotaWithStep(T begin, T end, T step) {
		auto count = static_cast<int>(std::ceil((end - begin) / step));
		std::vector<T> result;
		result.reserve(count);
		for (auto i = 0; i < count; ++i) {
			result.push_back(begin);
			begin += step;
		}
		return result;
	}

	bool checkData(const Json& aData) {
		auto result = true;
		result &= aData.contains("name") && aData["name"].is_string();
		result &= aData.contains("value") || aData.contains("values");
		if (aData.contains("values")) {
			auto size = 1;
			if (aData.contains("step")) {
				size = 2;
				result &= aData["step"].is_number();
			}
			result &= aData["values"].is_array() && aData["values"].size() >= size;
			if (aData.contains("step")) {
				for (const auto& value : aData["values"]) {
					result &= value.is_number();
				}
			}
		}

		if (aData.contains("criteria")) {
			result &= aData["criteria"].is_array() && aData.contains("otherwise");
			for (const auto& crit : aData["criteria"]) {
				result &= checkData(crit);
			}
			if (aData["criteria"].size() > 1) {
				result &= aData.contains("criteriaOperand") && aData["criteriaOperand"].is_string();
			}
		}
		return result;
	}
}

namespace calculation {
	template<typename algorithmDataType>
	class combinationFactory {
	public:
		combinationFactory(size_t aThreadsAmount) :
			threadsAmount(aThreadsAmount),
			combinationsData(aThreadsAmount),
			indexes(aThreadsAmount, 0)
		{
			settings = utils::readFromJson("input/combinationSettings");
			if (threadsAmount == 0 || settings.empty()) {
				return;
			}

			generateCombinations(0);
			utils::log("combinationFactory combinations - " + std::to_string(combinations));
			auto threadDataAmount = combinations / threadsAmount;
			auto lastThreadDataAmount = threadDataAmount + combinations % threadsAmount;
			std::shuffle(allData.begin(), allData.end(), std::default_random_engine{});
			for (size_t i = 0; i < threadsAmount; ++i) {
				const bool isLast = i == 0;
				const auto index = threadsAmount - 1 - i;
				const auto amount = (isLast) ? lastThreadDataAmount : threadDataAmount;
				combinationsData[index].reserve(amount);
				std::copy(allData.begin() + (allData.size() - amount), allData.end(), std::back_inserter(combinationsData[index]));
				allData.erase(allData.begin() + (allData.size() - amount), allData.end());
			}
			if (!allData.empty()) {
				utils::logError("combinationFactory tmpAllData not empty");
			}
		}
		size_t getCombinationsAmount() const { return combinations; }
		size_t getCurrentIndex() const { return std::accumulate(indexes.cbegin(), indexes.cend(), size_t{ 0 }); }
		const std::vector<algorithmDataType>& getThreadData(int aThread) { return combinationsData[aThread]; }
		void incrementThreadIndex(int aThread) { ++indexes[aThread]; }
		void onFinish() const {
			if (std::accumulate(indexes.begin(), indexes.end(), size_t{ 0 }) != combinations) {
				utils::logError("combinationFactory onFinish combinations not correct");
			}
		}
	private:
		void generateCombinations(size_t aIndex) {
			if (aIndex >= settings.size()) {
				onIterate();
				return;
			}
			auto iterationData = settings[aIndex];
			if (!checkData(iterationData)) {
				utils::logError("combinationFactory::generateCombinations wrong data");
				assert(false && "!checkData(iterationData)");
				return;
			}
			const auto name = iterationData["name"].get<std::string>();
			if (iterationData.contains("value")) {
				bool criteriaCheck = true;
				if (iterationData.contains("criteria")) {
					criteriaCheck = checkCriteria(iterationData["criteria"], iterationData["criteriaOperand"]);
				}
				iterateCombination(aIndex, name, (criteriaCheck) ? iterationData["value"] : iterationData["otherwise"]);
			}
			else {
				auto& values = iterationData["values"];
				bool criteriaCheck = true;
				if (iterationData.contains("criteria")) {
					criteriaCheck = checkCriteria(iterationData["criteria"], iterationData["criteriaOperand"]);
				}
				if (criteriaCheck) {
					if (iterationData.contains("step")) {
						auto step = iterationData["step"];
						if (step.is_number_float()) {
							const auto stepValue = step.get<double>();
							for (const auto& value : iotaWithStep(values.front().get<double>(), values.back().get<double>() + stepValue, stepValue)) {
								iterateCombination(aIndex, name, value);
							}
						}
						else {
							const auto stepValue = step.get<int>();
							for (const auto& value : iotaWithStep(values.front().get<int>(), values.back().get<int>() + stepValue, stepValue)) {
								iterateCombination(aIndex, name, value);
							}
						}
					}
					else {
						for (const auto& value : values) {
							iterateCombination(aIndex, name, value);
						}
					}
				}
				else {
					iterateCombination(aIndex, name, iterationData["otherwise"]);
				}
			}
		}
		void iterateCombination(size_t aIndex, const std::string& aName, const Json& aValue) {
			if (data.initDataField(aName, aValue)) {
				generateCombinations(aIndex + 1);
			}
			else {
				utils::logError("combinationFactory::iterateCombination wrong data field");
			}
		}
		void onIterate() {
			++combinations;
			if (!data.isValid()) {
				utils::logError("combinationFactory invalid algorithm data");
				assert(false && "!tmpData.isValid()");
			}
			else {
				allData.push_back(data);
			}
		}
		bool checkCriteria(const Json& aData, const Json& aOperand) {
			auto result = true;
			const std::string operand = (aOperand.is_string()) ? aOperand.get<std::string>() : "";
			if (operand == "or") {
				result = false;
			}
			else if (!operand.empty() && operand != "and") {
				utils::logError("checkCriteria wrond operand");
			}
			for (const auto& criteria : aData) {
				if (operand == "or") {
					result |= data.checkCriteria(criteria["name"].get<std::string>(), criteria["value"]);
				}
				else {
					result &= data.checkCriteria(criteria["name"].get<std::string>(), criteria["value"]);
				}
			}
			return result;
		}

		Json settings;
		std::vector<std::vector<algorithmDataType>> combinationsData;
		std::vector<algorithmDataType> allData;
		algorithmDataType data;
		std::vector<size_t> indexes;

		size_t combinations = 0;
		const size_t threadsAmount = 0;
	};
}
