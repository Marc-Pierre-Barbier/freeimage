cmake_minimum_required(VERSION 3.15)

project(freeimage C CXX)

file(GLOB_RECURSE SRC_LIST Source/*.cpp Source/transupp.c)
add_library(${PROJECT_NAME} ${SRC_LIST})

file(GLOB_RECURSE HEADER_LIST Source/*.h)
list(APPEND HEADER_LIST ${CMAKE_BINARY_DIR}/config.h)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADER_LIST}")

target_include_directories(${PROJECT_NAME} PUBLIC Source ${PROJECT_BINARY_DIR})

option(USE_ZUTIL "Use zutil.h for wider os support and zlib version support instead of hardcodded values (this requires exporting zutil.h from zlib)" OFF)
if(USE_ZUTIL)
    add_definitions(-DUSE_ZUTIL_H)
endif()

# you can require/disable them with these variables
# CMAKE_DISABLE_FIND_PACKAGE_<PackageName>
# CMAKE_REQUIRE_FIND_PACKAGE_<PackageName>

find_package(JPEG)
if(JPEG_FOUND)
    target_link_libraries(${PROJECT_NAME} JPEG::JPEG)
endif()

find_package(jxrlib)
if(jxrlib_FOUND)
    target_link_libraries(${PROJECT_NAME} jxrlib::jxrlib)
endif()

find_package(OpenJPEG)
if(OpenJPEG_FOUND)
    target_link_libraries(${PROJECT_NAME} openjp2)
endif()

find_package(PNG)
if(PNG_FOUND)
    target_link_libraries(${PROJECT_NAME} PNG::PNG)
endif()

find_package(libraw)
if(libraw_FOUND)
    target_link_libraries(${PROJECT_NAME} libraw::libraw)
endif()

find_package(TIFF)
#you need a tiff package with tiffio.h
if(TIFF_FOUND AND TIFFIOP_AVAILABLE)
    target_link_libraries(${PROJECT_NAME} TIFF::TIFF)
else()
    message(WARNING "disabling tiff conditions not met")
    unset(TIFF_FOUND)
endif()

find_package(WebP)
if(WebP_FOUND)
    target_link_libraries(${PROJECT_NAME} WebP::webp)
endif()

find_package(OpenEXR)
if(OpenEXR_FOUND)
    target_link_libraries(${PROJECT_NAME} OpenEXR::OpenEXR)
endif()

if(NOT BUILD_SHARED_LIBS)
    add_definitions(-DFREEIMAGE_LIB)
endif()

configure_file(Source/config.h.in config.h)

find_package(ZLIB REQUIRED)
target_link_libraries(${PROJECT_NAME} ZLIB::ZLIB)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Xclang -fnative-half-type")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -fnative-half-type")
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GCC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -O3 -fexceptions -fvisibility=hidden -ansi")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fexceptions -fvisibility=hidden -Wno-ctor-dtor-privacy -ansi")
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  /O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
endif()

install(TARGETS ${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

if(JPEG_FOUND AND OpenJPEG_FOUND AND PNG_FOUND AND TIFF_FOUND AND OpenEXR_FOUND)
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "Disabling, test suites due to missing formats JPEG:${JPEG_FOUND} - OpenJPEG:${OpenJPEG_FOUND} - PNG:${PNG_FOUND} - TIFF:${TIFF_FOUND} - OpenEXR:${OpenEXR_FOUND}")
endif()
