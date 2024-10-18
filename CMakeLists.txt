cmake_minimum_required(VERSION 3.19)

project(freeimage CXX)

file(GLOB_RECURSE SRC_LIST Source/**/*.cpp)
add_library(${PROJECT_NAME} ${SRC_LIST})

file(GLOB_RECURSE HEADER_LIST Source/*.h)
list(APPEND HEADER_LIST ${CMAKE_BINARY_DIR}/config.h)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADER_LIST}")

target_include_directories(${PROJECT_NAME} PRIVATE Source)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR})

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
    target_link_libraries(${PROJECT_NAME} OpenJPEG::OpenJPEG)
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
if(TIFF_FOUND)
    target_link_libraries(${PROJECT_NAME} TIFF::TIFF)
endif()

find_package(WebP)
if(WebP_FOUND)
    target_link_libraries(${PROJECT_NAME} WebP::webp)
endif()

find_package(OpenEXR)
if(OpenEXR_FOUND)
    target_link_libraries(${PROJECT_NAME} OpenEXR::OpenEXR)
endif()

add_definitions(-DFREEIMAGE_LIB)

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
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -O3 -fexceptions -fvisibility=hidden -ansi")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fexceptions -fvisibility=hidden")
endif()

install(TARGETS ${PROJECT_NAME})
