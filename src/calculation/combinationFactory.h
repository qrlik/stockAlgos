#pragma once
#include "algorithm/superTrend/stAlgorithmData.h"
#include <functional>
#include <optional>
#include <unordered_set>

namespace calculation {
	class combinationFactory {
	public:
		combinationFactory(size_t aThreadsAmount);
		size_t getCombinationsAmount() const;
		size_t getCurrentIndex() const;
		const std::vector<algorithm::stAlgorithmData>& getThreadData(int aThread);
		void incrementThreadIndex(int aThread);
		void onFinish();
	private:
		void generateCombinations(size_t aIndex);
		void iterateCombination(size_t aIndex, const std::string& aName, const Json& aValue);
		void onIterate();
		bool checkCriteria(const Json& aData, const Json& aOperand);

		Json settings;
		std::vector<std::vector<algorithm::stAlgorithmData>> combinationsData;
		std::vector<algorithm::stAlgorithmData> allData;
		algorithm::stAlgorithmData data;
		std::vector<size_t> indexes;

		size_t combinations = 0;
		const size_t threadsAmount = 0;
	};
}
