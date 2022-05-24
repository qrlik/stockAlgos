#pragma once
#include <numeric>

namespace algorithm {
	class stMAlgorithm;
	class maDirectionModule {
		friend class stMAlgorithm;
	public:
		maDirectionModule(stMAlgorithm& aAlgorithm);
		bool update();

		bool isFirstUp() const;
		bool isSecondUp() const;
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

		void updateData(maData& aData, double aMa, double aPresicion);

		stMAlgorithm& algorithm;

		maData firstData;
		maData secondData;
	};
}
