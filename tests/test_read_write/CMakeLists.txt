project(test_read_write)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE freeimage)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/../assets)

