cmake_minimum_required(VERSION 3.0.0)

# 3rd party modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake/Modules/")
# Enable fancy colours
set(CMAKE_COLOR_MAKEFILE ON)
# Enable verbose output for 'make'
#set(CMAKE_VERBOSE_MAKEFILE ON)

project(ranker)

# enable C++
enable_language(CXX)

# get the git revision
include(GetGitRevisionDescription)
git_describe(GIT_DESCRIBE)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

# Build with Clang/G++? -- currently does nothing, initialize with
#   $ CXX=clang++ cmake
#   $ CXX=g++ cmake

message("-- Default C++ compiler: ${CMAKE_CXX_COMPILER}")

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

# Boost
find_package(Boost)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# Flags
set(cxx_compiler_flags
  -pedantic-errors
  -Wextra
  -Wall
  -Wfloat-equal
  -fdiagnostics-show-option
  # -march=native
  -std=c++17
  -Wctor-dtor-privacy
  -Weffc++
  # -fPIC
  -fno-strict-aliasing
  -Woverloaded-virtual
  # -Wold-style-cast
)

foreach(flag ${cxx_compiler_flags})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
endforeach()

# generate the file with version names
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.cpp.in
   ${CMAKE_CURRENT_BINARY_DIR}/version.cpp @ONLY)

add_library(complement STATIC
# add_library(complement SHARED
	Algorithms/AuxFunctions.cpp
	Algorithms/Simulations.cpp
	Automata/AutGraph.cpp
	Automata/AutomatonStruct.cpp
	Automata/BuchiAutomataParser.cpp
	Automata/BuchiAutomaton.cpp
	Automata/GenCoBuchiAutomaton.cpp
	Complement/BuchiAutomatonSpec.cpp
	Complement/BuchiDelay.cpp
	Complement/CoBuchiCompl.cpp
	Complement/ElevatorAutomaton.cpp
	Complement/GenCoBuchiAutomatonCompl.cpp
	Complement/RankFunc.cpp
	Debug/BuchiAutomatonDebug.cpp
	Ranker-general.cpp
)

add_executable(ranker
	ranker.cpp
	${CMAKE_CURRENT_BINARY_DIR}/version.cpp
)

# add_executable(test-parser units/test-parser.cpp)
# add_executable(test-classify units/test-classify.cpp)
# add_executable(compl-check units/compl-check.cpp)

target_link_libraries(ranker complement)
# target_link_libraries(compl-check complement)

# add_test(
# 	NAME unit_tests
# 	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/compl-check
# 	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
# )

include(CTest)
enable_testing()
