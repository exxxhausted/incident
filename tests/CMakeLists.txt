find_package(Catch2 3 QUIET)

if(NOT Catch2_FOUND)
    message(STATUS "Catch2 не найден, скачиваю через FetchContent")
    include(FetchContent)
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)
endif()

file(GLOB TEST_SOURCES "*.cpp")
add_executable(incident_tests ${TEST_SOURCES})

target_link_libraries(incident_tests PRIVATE
    incident
    Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(incident_tests)
