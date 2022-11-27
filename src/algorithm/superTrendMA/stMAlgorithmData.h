#pragma once
#include "algorithm/algorithmDataBase.h"

namespace algorithm {
	class stMAlgorithmData final : public algorithmDataBase {
	public:
		stMAlgorithmData(const std::string ticker);
		bool operator==(const stMAlgorithmData& aOther) const;

		size_t getCustomID() const override;

	protected:
		size_t getCustomHash() const override;

	private:
		bool initDataFieldInternal(const std::string& aName, const Json& aValue) override;
		bool checkCriteriaInternal(const std::string& aName, const Json& aValue) const override;
		bool isValidInternal() const override;
		void addJsonDataInternal(Json& aData) const override;

	};
}