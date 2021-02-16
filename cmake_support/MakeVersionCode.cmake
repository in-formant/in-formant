function(leftpadzeroes out_var value)
    string(LENGTH "${value}" value_length)
    if (value_length EQUAL 1)
        set(${out_var} "0${value}" PARENT_SCOPE)
    else()
        set(${out_var} "${value}" PARENT_SCOPE)
    endif()
endfunction()

set(VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(VERSION_PATCH ${PROJECT_VERSION_PATCH})

leftpadzeroes(VERSION_MAJOR_PAD ${VERSION_MAJOR})
leftpadzeroes(VERSION_MINOR_PAD ${VERSION_MINOR})
leftpadzeroes(VERSION_PATCH_PAD ${VERSION_PATCH})

set(VERSION_CODE "99${VERSION_MAJOR_PAD}${VERSION_MINOR_PAD}${VERSION_PATCH_PAD}")
