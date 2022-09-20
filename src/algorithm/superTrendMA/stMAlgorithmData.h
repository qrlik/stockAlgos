#pragma once
#include "algorithm/algorithmDataBase.h"

namespace algorithm {
	class stMAlgorithmData final : public algorithmDataBase {
	public:
		stMAlgorithmData(const std::string ticker);
		bool operator==(const stMAlgorithmData& aOther) const;

		size_t getCustomID() const override;

		double getFirstMATrendPrecision() const { return firstMATrendPrecision; }
		double getSecondMATrendPrecision() const { return secondMATrendPrecision; }
		double getCloserMATrendPrecision() const { return closerMATrendPrecision; }
		double getCloserTrailPrecision() const { return closerTrailPrecision; }
		double getActivationPercent() const { return activationPercent; }

		bool getCloserTrendChangeCheck() const { return closerTrendChangeCheck; }
		bool getCloserMACheck() const { return closerMACheck; }
		bool getCloserConjuctionCheck() const { return closerConjuctionCheck; }
		bool getCloserTrailStop() const { return closerTrailStop; }

	protected:
		size_t getCustomHash() const override;

	private:
		bool initDataFieldInternal(const std::string& aName, const Json& aValue) override;
		bool checkCriteriaInternal(const std::string& aName, const Json& aValue) const override;
		bool isValidInternal() const override;
		void addJsonDataInternal(Json& aData) const override;

		double firstMATrendPrecision = -1.0;
		double secondMATrendPrecision = -1.0;
		double closerMATrendPrecision = -1.0;
		double closerTrailPrecision = -1.0;
		double activationPercent = -1.0;
		
		bool closerTrendChangeCheck = false;
		bool closerMACheck = false;
		bool closerTrailStop = false;
		bool closerConjuctionCheck = false;
	};
}