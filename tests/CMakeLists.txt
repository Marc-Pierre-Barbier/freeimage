
set(test_assets "${CMAKE_CURRENT_LIST_DIR}/assets")

add_subdirectory(dominique_test)
add_subdirectory(test_read_write)

if(WebP_FOUND AND jxrlib_FOUND AND libraw_FOUND)
    add_subdirectory(original)
else()
    message(STATUS "Disabling, FreeImage's default test suite due to missing formats (ALL of them are required)")
endif()
