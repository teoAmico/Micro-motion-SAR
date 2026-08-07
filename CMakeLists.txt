cmake_minimum_required(VERSION 3.16)
project(micromotion C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# -ffast-math is deliberately absent. It permits reassociation and flushes
# denormals, which perturbs sub-pixel correlation peaks and interferometric
# phase -- the exact quantities this project measures. Do not add it.
add_compile_options(-Wall -Wextra -Wshadow -Wconversion -Wno-sign-conversion)

set(CMAKE_C_FLAGS_ASAN "-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS_ASAN "-fsanitize=address,undefined")

find_package(OpenMP COMPONENTS C)
if(NOT OpenMP_C_FOUND AND APPLE)
    foreach(p /opt/homebrew/opt/libomp /usr/local/opt/libomp)
        if(EXISTS ${p})
            set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -I${p}/include")
            set(OpenMP_C_LIB_NAMES "omp")
            set(OpenMP_omp_LIBRARY "${p}/lib/libomp.dylib")
            find_package(OpenMP COMPONENTS C)
        endif()
    endforeach()
endif()
if(OpenMP_C_FOUND)
    message(STATUS "OpenMP enabled")
else()
    message(WARNING "OpenMP NOT found -- backprojection will run on one core")
endif()

add_library(pocketfft STATIC third_party/pocketfft/pocketfft.c)
target_include_directories(pocketfft PUBLIC third_party/pocketfft)
target_compile_options(pocketfft PRIVATE -w)

add_library(mm_core STATIC
    src/util/status.c
    src/util/geom.c
    src/util/geocode.c
    src/util/orbit.c
    src/core/fft.c
    src/core/focus.c
    src/core/subaperture.c
    src/core/coreg.c
    src/core/phaselink.c
    src/core/microm.c
    src/core/ccd.c
    src/core/validate.c
    src/core/spectrum.c
    src/core/modalfit.c
    src/core/simulate.c
    src/readers/uavsar.c
    src/readers/cphd.c
    src/readers/sicd.c
    src/io/raster.c
    src/io/png.c
    src/io/figure.c
)
target_include_directories(mm_core PUBLIC include)
target_link_libraries(mm_core PRIVATE pocketfft)
target_link_libraries(mm_core PUBLIC m)
if(OpenMP_C_FOUND)
    target_link_libraries(mm_core PUBLIC OpenMP::OpenMP_C)
endif()

add_executable(micromotion src/main.c)
target_link_libraries(micromotion PRIVATE mm_core)
foreach(t sim_cphd sim_slc crop_slc)
    add_executable(${t} tools/${t}.c)
    target_link_libraries(${t} PRIVATE mm_core)
endforeach()

enable_testing()
add_test(NAME sim_cphd_rejects_missing_option_value
         COMMAND sim_cphd ignored.cphd --clutter)
set_tests_properties(sim_cphd_rejects_missing_option_value PROPERTIES WILL_FAIL TRUE)
add_test(NAME sim_cphd_rejects_unknown_option
         COMMAND sim_cphd ignored.cphd --cluter 3000)
set_tests_properties(sim_cphd_rejects_unknown_option PROPERTIES WILL_FAIL TRUE)

# --json is a machine interface, so its refusal path is part of the contract:
# a source it does not support must fail rather than fall back to the human
# report, which a parser would either choke on or silently misread.
add_test(NAME info_json_refuses_non_cphd
         COMMAND micromotion info --sicd ignored.nitf --json)
set_tests_properties(info_json_refuses_non_cphd PROPERTIES WILL_FAIL TRUE)

set(MM_TESTS
    test_constants test_fft test_focus test_subaperture test_coreg
    test_cullsweep test_modalfit test_modalset
    test_endtoend test_tracking test_nullmotion test_ccd test_validate
    test_pairedecho test_phaselink test_cphd test_readers test_geocode test_raster
)
foreach(t IN LISTS MM_TESTS)
    add_executable(${t} tests/${t}.c)
    target_link_libraries(${t} PRIVATE mm_core)
    target_include_directories(${t} PRIVATE tests)
    add_test(NAME ${t} COMMAND ${t})
endforeach()

# The comment rule is enforced over tests/ as well as src/ and tools/. It was
# src/ and tools/ only, which left the shared fixture generator and the scoring
# harness -- the code every measurement in FOLLOW-UPS.md was taken through --
# outside the rule that exists to keep units straight. See docs/CODE-REVIEW.md
# finding 8.
find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    add_test(NAME docs_coverage
             COMMAND ${Python3_EXECUTABLE}
                     ${CMAKE_SOURCE_DIR}/scripts/check_docs.py
                     ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/tools
                     ${CMAKE_SOURCE_DIR}/tests)
else()
    # Not a silent skip: without this the suite quietly becomes 20 tests while
    # ctest still reports success, and both CLAUDE.md and the user guide say 21.
    message(WARNING
            "Python3 not found -- the docs_coverage test will NOT run, so the "
            "block-comment rule is unenforced and 'ctest' reports 20 tests "
            "rather than 21.")
endif()
