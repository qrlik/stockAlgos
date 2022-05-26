#pragma once
#include "algorithm/algorithmDataBase.h"

namespace algorithm {
	class stMAlgorithmData final : public algorithmDataBase {
	public:
		stMAlgorithmData();

		double getFirstMATrendPrecision() const { return firstMATrendPrecision; }
		double getSecondMATrendPrecision() const { return secondMATrendPrecision; }

		bool getCloserTrendChangeCheck() const { return closerTrendChangeCheck; }
		bool getCloserMainMACheck() const { return closerMainMACheck; }
		bool getCloserConjuctionCheck() const { return closerConjuctionCheck; }

	private:
		bool initDataFieldInternal(const std::string& aName, const Json& aValue) override;
		bool checkCriteriaInternal(const std::string& aName, const Json& aValue) const override;
		bool isValidInternal() const override;
		void addJsonDataInternal(Json& aData) const override;

		double firstMATrendPrecision = 0.0;
		double secondMATrendPrecision = 0.0;
		
		bool closerTrendChangeCheck = false;
		bool closerMainMACheck = false;
		bool closerConjuctionCheck = false;
	};
}