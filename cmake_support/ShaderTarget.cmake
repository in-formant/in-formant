set(GLSLC_DIR "/usr/local/bin")
if(DEFINED ENV{GLSLC_DIR})
    set(GLSLC_DIR $ENV{GLSLC_DIR})
endif()

set(GLSL_VALIDATOR "${GLSLC_DIR}/glslangValidator")
set(GLSLC          "${GLSLC_DIR}/glslc")

if(NOT EXISTS ${GLSL_VALIDATOR})
    message(FATAL_ERROR "Could not find glslangValidator at ${GLSL_VALIDATOR}")
endif()

if(NOT EXISTS ${GLSLC})
    message(FATAL_ERROR "Could not find glslc at ${GLSLC}")
endif()

if(DEFINED ENV{GLSLC_GLIBC_DIR})
    set(COMMAND_PREFIX export LD_LIBRARY_PATH="$ENV{GLSLC_GLIBC_DIR}/lib:$$LD_LIBRARY_PATH" && )
else()
    set(COMMAND_PREFIX "")
endif()

function(add_shader_dependencies)
    cmake_parse_arguments(PARSED_ARGS "" "TARGET;SEMANTICS" "SOURCES" ${ARGN})

    set(SOURCE_TARGET_NAME ${PARSED_ARGS_TARGET})
    set(GLSL_SOURCE_FILES ${PARSED_ARGS_SOURCES})
    set(GLSL_SEMANTICS ${PARSED_ARGS_SEMANTICS})

    if(GLSL_SEMANTICS STREQUAL "OpenGL")
        set(target_suffix "opengl")
        set(output_subdir "opengl")
        set(is_compiled TRUE)
        set(glslc_arg "-G")
    elseif(GLSL_SEMANTICS STREQUAL "GLES")
        set(target_suffix "gles")
        set(output_subdir "gles")
        set(is_compiled FALSE)
    elseif(GLSL_SEMANTICS STREQUAL "Vulkan")
        set(target_suffix "vulkan")
        set(output_subdir "vulkan")
        set(is_compiled TRUE)
        set(glslc_arg "-V")
    else()
        message(FATAL_ERROR "Did not recognize GLSL semantics: ${GLSL_SEMANTICS}")
    endif()

    set(output_target_name "${SOURCE_TARGET_NAME}_shaders_${target_suffix}")
    set(output_dir "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${output_target_name}.dir/shaders/${output_subdir}")

    set(output_files "")

    add_custom_command(
        TARGET ${SOURCE_TARGET_NAME}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${output_dir}"
    )

    if(is_compiled)
        foreach(glsl_file ${GLSL_SOURCE_FILES})
            get_filename_component(glsl_file_name ${glsl_file} NAME)
            set(spirv_file "${output_dir}/${glsl_file_name}.spv")
            add_custom_command(
                OUTPUT ${spirv_file}
                COMMAND ${CMAKE_COMMAND} -E make_directory "${output_dir}"
                COMMAND ${CMAKE_COMMAND} -E remove ${spirv_file}
                COMMAND ${COMMAND_PREFIX} ${GLSL_VALIDATOR} ${glslc_arg} ${glsl_file} -o ${spirv_file}
                DEPENDS ${glsl_file}
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            )
            list(APPEND output_files "${spirv_file}")
        endforeach()
    else()
        foreach(glsl_file ${GLSL_SOURCE_FILES})
            get_filename_component(glsl_file_name ${glsl_file} NAME)
            set(copied_glsl_file "${output_dir}/${glsl_file_name}")
            add_custom_command(
                OUTPUT ${copied_glsl_file}
                COMMAND ${CMAKE_COMMAND} -E make_directory "${output_dir}"
                COMMAND ${CMAKE_COMMAND} -E remove ${copied_glsl_file}
                COMMAND ${CMAKE_COMMAND} -E copy ${glsl_file} ${copied_glsl_file}
                DEPENDS ${glsl_file}
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            )
            list(APPEND output_files "${copied_glsl_file}")
        endforeach()
    endif()

    add_custom_target(${output_target_name} DEPENDS ${output_files} SOURCES ${GLSL_SOURCE_FILES})
    add_dependencies(${SOURCE_TARGET_NAME} ${output_target_name})
    add_custom_command(
        TARGET ${SOURCE_TARGET_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:${SOURCE_TARGET_NAME}>/shaders/${output_subdir}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${SOURCE_TARGET_NAME}>/shaders/${output_subdir}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${output_dir}" "$<TARGET_FILE_DIR:${SOURCE_TARGET_NAME}>/shaders/${output_subdir}"
        DEPENDS ${GLSL_SOURCE_FILES}
    )

endfunction()
