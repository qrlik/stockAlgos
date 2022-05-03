#include "combinationFactory.h"
#include "market/marketRules.h"
#include "tests/checkers.hpp"
#include "utils/utils.h"
#include <iostream>

using namespace calculation;

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

	std::vector<double> getLiquidationRange(int aLeverage, double aMargin, int aSteps, double aMinOffset) {
		std::vector<double> result;
		auto liqPercent = MARKET_DATA->getLiquidationPercent(aMargin, aLeverage);
		if (aSteps > 0) {
			auto stepSize = liqPercent / (aSteps + 1);
			for (auto i = 0; i < aSteps; ++i) {
				liqPercent -= stepSize;
				result.push_back(liqPercent);
			}
		}
		result.push_back(aMinOffset);
		return result;
	}

	std::vector<algorithm::stAlgorithmData> tmpAllData;
	algorithm::stAlgorithmData tmpData;
}

combinationFactory::combinationFactory(size_t aThreadsAmount) :
	threadsAmount(aThreadsAmount),
	combinationsData(aThreadsAmount),
	indexes(aThreadsAmount, 0)
{
	settings = utils::readFromJson("combinationSettings");
	if (threadsAmount == 0 || settings.empty()) {
		return;
	}

	generateCombinations(0);
	utils::log("combinationFactory combinations - " + std::to_string(combinations));
	auto threadDataAmount = combinations / threadsAmount;
	auto lastThreadDataAmount = threadDataAmount + combinations % threadsAmount;
	for (size_t i = 0; i < threadsAmount; ++i) {
		const bool isLast = i == 0;
		const auto index = threadsAmount - 1 - i;
		const auto amount = (isLast) ? lastThreadDataAmount : threadDataAmount;
		combinationsData[index].reserve(amount);
		std::copy(tmpAllData.begin() + (tmpAllData.size() - amount), tmpAllData.end(), std::back_inserter(combinationsData[index]));
		tmpAllData.erase(tmpAllData.begin() + (tmpAllData.size() - amount), tmpAllData.end());
	}
	if (!tmpAllData.empty()) {
		utils::logError("combinationFactory tmpAllData not empty");
	}
}

namespace {
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
				result |= tmpData.checkCriteria(criteria["name"].get<std::string>(), criteria["value"]);
			}
			else {
				result &= tmpData.checkCriteria(criteria["name"].get<std::string>(), criteria["value"]);
			}
		}
		return result;
	}
}

void combinationFactory::iterateCombination(size_t aIndex, const std::string& aName, const Json& aValue) {
	if (tmpData.initDataField(aName, aValue)) {
		generateCombinations(aIndex + 1);
	}
	else {
		utils::logError("combinationFactory::iterateCombination wrong data field");
	}
}

void combinationFactory::generateCombinations(size_t aIndex) {
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
		iterateCombination(aIndex, name, iterationData["value"]);
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

size_t combinationFactory::getCombinationsAmount() const {
	return combinations;
}

size_t combinationFactory::getCurrentIndex() const {
	return std::accumulate(indexes.cbegin(), indexes.cend(), size_t{ 0 });
}

const std::vector<algorithm::stAlgorithmData>& combinationFactory::getThreadData(int aThread) {
	return combinationsData[aThread];
}

void combinationFactory::incrementThreadIndex(int aThread) {
	++indexes[aThread];
}

void combinationFactory::onFinish() {
	if (std::accumulate(indexes.begin(), indexes.end(), size_t{ 0 }) != combinations) {
		utils::logError("combinationFactory onFinish combinations not correct");
	}
}

void combinationFactory::onIterate() {
	++combinations;
	if (!tmpData.isValid()) {
		assert(false && "!tmpData.isValid()");
		utils::logError("combinationFactory invalid algorithm data");
	}
	else {
		tmpAllData.push_back(tmpData);
	}
}

