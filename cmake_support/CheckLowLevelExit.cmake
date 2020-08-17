include(CheckSymbolExists)
include(CheckFunctionExists)

function(check_low_level_exit OUT_VAR)
    cmake_parse_arguments(FUNCTION "" "" "INCLUDES;NAMES" ${ARGN})
    
    unset(found_function_name)

    foreach(function_name ${FUNCTION_NAMES})
        set(var_symbol   ${var}_symbol)
        set(var_function ${var}_function)
       
        message(CHECK_START "Looking for ${function_name}")

        set(CMAKE_REQUIRED_QUIET TRUE)
        check_symbol_exists(${function_name} "${FUNCTION_INCLUDES}" ${var_symbol})
        check_function_exists(${function_name} ${var_function})

        if(${var_symbol} OR ${var_function})
            message(CHECK_PASS "found")
            set(found_function_name ${function_name})
            break()
        else()
            message(CHECK_FAIL "not found")
        endif()
    endforeach()

    if (DEFINED found_function_name)
        message(STATUS "Found low-level exit function: ${found_function_name}")
        set(${OUT_VAR} ${found_function_name} PARENT_SCOPE)
    else()
        message(FATAL_ERROR "No suitable low-level exit function")
    endif()
endfunction()


