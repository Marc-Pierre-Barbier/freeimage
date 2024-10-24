project(test)
file(GLOB_RECURSE TST_SRC_LIST tests/original/*.cpp)

add_executable(${PROJECT_NAME} ${TST_SRC_LIST})
target_link_libraries(${PROJECT_NAME} PRIVATE freeimage )
add_dependencies(${PROJECT_NAME} freeimage)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
file(COPY tests/original/exif.jpg DESTINATION "${CMAKE_BINARY_DIR}")
file(COPY tests/original/exif.jxr DESTINATION "${CMAKE_BINARY_DIR}")

if(unix)
	file(COPY tests/original/run_original_tests.sh DESTINATION "${CMAKE_BINARY_DIR}")
else()
	file(COPY tests/original/run_original_tests.bat DESTINATION "${CMAKE_BINARY_DIR}")
endif()
