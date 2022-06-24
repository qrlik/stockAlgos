#pragma once
#include "json/json.hpp"
#include <numeric>
#include <string>

namespace algorithm {
	class stMAlgorithm;
	class maDirectionModule {
		friend class stMAlgorithm;
	public:
		maDirectionModule(stMAlgorithm& aAlgorithm);
		bool operator==(const maDirectionModule& aOther) const;
		bool update();

		bool isFirstUp() const;
		bool isSecondUp() const;
		bool isCloserUp() const;

	private:
		enum class eMaState : signed char {
			NONE = -1,
			DOWN = 0,
			UP = 1
		};
		struct maData {
			double lastMinimum = std::numeric_limits<double>::max();
			double lastMaximum = 0.0;
			eMaState state = eMaState::NONE;
			bool isLastMaximum = false;
		};

		eMaState stateFromStr(const std::string aName) const;
		void updateData(maData& aData, double aMa, double aPresicion);
		void updateData(maData& aData, const Json& aJson);
		void updateCloserDiscrepancy();

		stMAlgorithm& algorithm;

		maData firstData;
		maData secondData;
		maData closerData;
	};
}
