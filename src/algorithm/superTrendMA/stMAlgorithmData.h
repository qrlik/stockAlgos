#pragma once
#include "algorithm/algorithmDataBase.h"

namespace algorithm {
	class stMAlgorithmData final : public algorithmDataBase {
	public:
		stMAlgorithmData();
		bool operator==(const stMAlgorithmData& aOther) const;

		double getFirstMATrendPrecision() const { return firstMATrendPrecision; }
		double getSecondMATrendPrecision() const { return secondMATrendPrecision; }
		double getCloserMATrendPrecision() const { return closerMATrendPrecision; }
		double getCloserTrailPrecision() const { return closerTrailPrecision; }

		bool getCloserTrendChangeCheck() const { return closerTrendChangeCheck; }
		bool getCloserMACheck() const { return closerMACheck; }
		bool getCloserConjuctionCheck() const { return closerConjuctionCheck; }
		bool getCloserTrailStop() const { return closerTrailStop; }

	private:
		bool initDataFieldInternal(const std::string& aName, const Json& aValue) override;
		bool checkCriteriaInternal(const std::string& aName, const Json& aValue) const override;
		bool isValidInternal() const override;
		void addJsonDataInternal(Json& aData) const override;

		double firstMATrendPrecision = 0.0;
		double secondMATrendPrecision = 0.0;
		double closerMATrendPrecision = 0.0;
		double closerTrailPrecision = 0.0;
		
		bool closerTrendChangeCheck = false;
		bool closerMACheck = false;
		bool closerTrailStop = false;
		bool closerConjuctionCheck = false;
	};
}