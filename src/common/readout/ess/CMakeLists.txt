#=============================================================================
# ESS Readout System Tests
#=============================================================================

set(ReadoutParserTest_INC Parser.h ParserTestData.h)
set(ReadoutParserTest_SRC ParserTest.cpp Parser.cpp)
create_test_executable(ReadoutParserTest)
