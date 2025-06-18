cmake_minimum_required(VERSION 3.14)
project(signal VERSION 1.0 LANGUAGES CXX)

include(FetchContent)

# Fetch delegate
FetchContent_Declare(
	delegate
	GIT_REPOSITORY https://github.com/nikitf777/delegate.git
	GIT_TAG main
)

FetchContent_MakeAvailable(delegate)

# Create the header-only library interface
add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} INTERFACE delegate)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

# Include directories for signal
target_include_directories(${PROJECT_NAME} INTERFACE
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:include>
)


# Install the header-only library target
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets)

# Install headers
install(DIRECTORY include/ DESTINATION include)

# Install the CMake package configuration
install(EXPORT ${PROJECT_NAME}Targets
	FILE ${PROJECT_NAME}Config.cmake
	DESTINATION lib/cmake/${PROJECT_NAME}
)

enable_testing()

find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME}-tests tests/tests.cpp)
target_compile_features(${PROJECT_NAME}-tests PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME}-tests PRIVATE
	GTest::GTest
	GTest::Main
	gmock
	${PROJECT_NAME})

add_test(NAME tests COMMAND tests)
