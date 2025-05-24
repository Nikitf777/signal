cmake_minimum_required(VERSION 3.14)
project(signal VERSION 1.0 LANGUAGES CXX)

# Include delegate as a subdirectory
# add_subdirectory(delegate)

include(FetchContent)

# Fetch delegate
FetchContent_Declare(
	delegate
	GIT_REPOSITORY https://github.com/nikitf777/delegate.git
	GIT_TAG v1.0.4
)
FetchContent_Declare(
	cppcoro
	GIT_REPOSITORY https://github.com/andreasbuhr/cppcoro.git
	GIT_TAG 6c9c0cea4a54d427e9ca7153f27baaae19039439
)

FetchContent_MakeAvailable(delegate)
FetchContent_MakeAvailable(cppcoro)

# Create the header-only library interface
add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} INTERFACE delegate)
target_link_libraries(${PROJECT_NAME} INTERFACE cppcoro)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

# Include directories for signal
target_include_directories(${PROJECT_NAME} INTERFACE
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
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
target_link_libraries(${PROJECT_NAME}-tests PRIVATE GTest::GTest GTest::Main)
target_link_libraries(${PROJECT_NAME}-tests PRIVATE gmock)
target_link_libraries(${PROJECT_NAME}-tests PRIVATE ${PROJECT_NAME})

add_test(NAME tests COMMAND tests)
