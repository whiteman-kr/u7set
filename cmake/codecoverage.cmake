cmake_minimum_required(VERSION 3.16)

function(add_code_coverage_options TARGET ENABLE_COVERAGE)
    # For GCC
    #
    if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")    
        target_compile_options(${TARGET} PUBLIC --coverage)
        target_link_options(${TARGET} PUBLIC --coverage)
        target_link_libraries(${TARGET} PUBLIC gcov)
    endif()
endfunction()