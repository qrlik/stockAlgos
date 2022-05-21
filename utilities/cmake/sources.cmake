include (FindPackageHandleStandardArgs)

file(GLOB_RECURSE SRC_ALGORITHM
    "src/algorithm/*.cpp"
    "src/algorithm/*.h"
    "src/algorithm/*.hpp"
    "src/algorithm/superTrend/*.cpp"
    "src/algorithm/superTrend/*.h"
    "src/algorithm/superTrend/*.hpp"
    "src/algorithm/superTrendMA/*.hpp"
    "src/algorithm/superTrendMA/*.hpp"
    "src/algorithm/superTrendMA/*.hpp"
)
file(GLOB_RECURSE SRC_CALCULATION
    "src/calculation/*.cpp"
    "src/calculation/*.h"
    "src/calculation/*.hpp"
)
file(GLOB_RECURSE SRC_CORE
    "src/core/*.cpp"
    "src/core/*.h"
    "src/core/*.hpp"
)
file(GLOB_RECURSE SRC_MARKET
    "src/market/*.cpp"
    "src/market/*.h"
    "src/market/*.hpp"
)
file(GLOB_RECURSE SRC_STRUCTS
    "src/structs/*.cpp"
    "src/structs/*.h"
    "src/structs/*.hpp"
)
file(GLOB_RECURSE SRC_TESTS
    "src/tests/*.cpp"
    "src/tests/*.h"
    "src/tests/*.hpp"
)
file(GLOB_RECURSE SRC_UTILS
    "src/utils/*.cpp"
    "src/utils/*.h"
    "src/utils/*.hpp"
)
file(GLOB_RECURSE SRC_JSON
    "src/json/*.hpp"
)

set(
    SOURCES
    ${SOURCES}
    ${SRC_ALGORITHM}
    ${SRC_CALCULATION}
    ${SRC_CORE}
    ${SRC_MARKET}
    ${SRC_STRUCTS}
    ${SRC_TESTS}
    ${SRC_UTILS}
    ${SRC_JSON}
)
