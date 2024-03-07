# Copyright (C) 2022 - 2023 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# bifrost bragg detector module and base test
#=============================================================================

add_subdirectory(generators)
add_subdirectory(test)


set(bragg_SRC main.cpp)
set(bragg_LIB efu_reduction caen_common efu_essreadout)
create_executable(bragg)
