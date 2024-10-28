project(dominic_test)

add_executable(${PROJECT_NAME} tests/dominique_s_test_suite/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE freeimage)
add_dependencies(${PROJECT_NAME} freeimage)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
file(COPY tests/dominique_s_test_suite/tests_in DESTINATION "${CMAKE_BINARY_DIR}")
file(COPY tests/dominique_s_test_suite/tests_out DESTINATION "${CMAKE_BINARY_DIR}")

if(UNIX)
	file(COPY tests/dominique_s_test_suite/run_dominique_tests.sh DESTINATION "${CMAKE_BINARY_DIR}")
else()
	file(COPY tests/dominique_s_test_suite/run_dominique_tests.bat DESTINATION "${CMAKE_BINARY_DIR}")
endif()
