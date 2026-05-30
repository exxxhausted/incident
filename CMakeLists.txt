cmake_minimum_required(VERSION 3.16)

project(Incident
    VERSION 0.0.1
    LANGUAGES CXX)

add_library(incident INTERFACE)
target_include_directories(incident INTERFACE include)
target_compile_features(incident INTERFACE cxx_std_23)

option(BUILD_EXAMPLES "Build examples" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

option(BUILD_SINGLE_HEADER "Generate single-header version" ON)
if(BUILD_SINGLE_HEADER)
    set(INCIDENT_MAIN_HEADER "${CMAKE_SOURCE_DIR}/include/incident/incident.hpp")
    set(SINGLE_HEADER_OUTPUT "${CMAKE_SOURCE_DIR}/single_include/incident.hpp")

    add_custom_target(single_header ALL
        COMMAND quom ${INCIDENT_MAIN_HEADER} ${SINGLE_HEADER_OUTPUT}
        COMMENT "Generating single-header: ${SINGLE_HEADER_OUTPUT}"
    )
endif()
