#=============================================================================
# Tests
#=============================================================================

include_directories(.)
include_directories(..)

get_filename_component(CAEN_FULL "${ESS_MODULE_DIR}/loki/configs/Loki.json" ABSOLUTE)

##
## Pure unit tests
##

set(CspecGeometryTest_INC
        ../geometry/CspecGeometry.h
    ${ESS_MODULE_DIR}/caen/geometry/Config.h
    ${ESS_MODULE_DIR}/caen/geometry/Calibration.h
)
set(CspecGeometryTest_SRC
        CspecGeometryTest.cpp
        ../geometry/CspecGeometry.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Config.cpp
    ${ESS_MODULE_DIR}/caen/geometry/Calibration.cpp
)
create_test_executable(CspecGeometryTest)

