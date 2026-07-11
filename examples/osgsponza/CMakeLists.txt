set(TARGET_SRC
    osgsponza.cpp
    SponzaOptions.cpp
    SponzaCameras.cpp
    SponzaTargets.cpp
    ShadowPass.cpp
    SponzaRayScene.cpp
    SponzaGpuRayScene.cpp
    FullscreenQuad.cpp
    MainPass.cpp
    SsaoPass.cpp
    TonemapPass.cpp
    SponzaLighting.cpp
    SponzaVisibilityBake.cpp)
setup_example(osgsponza)

set(OSGSPONZA_PROFILE_CAMERA_INDEX "0" CACHE STRING
    "Camera index for the osgsponza profiling workload")
set(OSGSPONZA_PROFILE_RENDER_SCALE "1" CACHE STRING
    "Headless render scale for the osgsponza profiling workload")
set(OSGSPONZA_PROFILE_OUTPUT_WIDTH "1280" CACHE STRING
    "Output width for the osgsponza profiling workload")
set(OSGSPONZA_PROFILE_OUTPUT_HEIGHT "720" CACHE STRING
    "Output height for the osgsponza profiling workload")
set(OSGSPONZA_PROFILE_RT_SHADOWS "off" CACHE STRING
    "Enable RT shadows for the osgsponza profiling workload")
set(OSGSPONZA_PROFILE_RT_SHADOW_SAMPLES "1" CACHE STRING
    "RT shadow sample count for the osgsponza profiling workload")
set(OSGSPONZA_PROFILE_BENCHMARK_FRAMES "120" CACHE STRING
    "Headless benchmark frames for the osgsponza profiling workload")
set(OSGSPONZA_PROFILE_BENCHMARK_WARMUP_FRAMES "10" CACHE STRING
    "Headless benchmark warmup frames for the osgsponza profiling workload")
set(OSGSPONZA_PROFILE_EXTRA_ARGS "--ssao off --fxaa off --shadow-map-size 2048"
    CACHE STRING
    "Extra command-line arguments appended to the osgsponza profiling workload")
set(OSGSPONZA_PROFILE_DIR "${CMAKE_BINARY_DIR}/profiles/osgsponza" CACHE PATH
    "Output directory for osgsponza profiling artifacts")

set(_osgsponza_data_dir "${PROJECT_SOURCE_DIR}/../workspace/sponza/main_sponza")
set(_osgsponza_texture_dir "${_osgsponza_data_dir}/textures")
set(_osgsponza_profile_output
    "${OSGSPONZA_PROFILE_DIR}/camera-${OSGSPONZA_PROFILE_CAMERA_INDEX}.png")
separate_arguments(_osgsponza_profile_extra_args
    UNIX_COMMAND
    "${OSGSPONZA_PROFILE_EXTRA_ARGS}")

set(_osgsponza_profile_command
    ${CMAKE_COMMAND} -E env
    "OSG_LIBRARY_PATH=${OUTPUT_LIBDIR}"
    "OSG_FILE_PATH=${_osgsponza_data_dir}:${_osgsponza_texture_dir}"
    $<TARGET_FILE:example_osgsponza>
    --headless "${_osgsponza_profile_output}"
    --camera-index "${OSGSPONZA_PROFILE_CAMERA_INDEX}"
    --rt-shadows "${OSGSPONZA_PROFILE_RT_SHADOWS}"
    --rt-shadow-samples "${OSGSPONZA_PROFILE_RT_SHADOW_SAMPLES}"
    --render-scale "${OSGSPONZA_PROFILE_RENDER_SCALE}"
    --output-size "${OSGSPONZA_PROFILE_OUTPUT_WIDTH}" "${OSGSPONZA_PROFILE_OUTPUT_HEIGHT}"
    --benchmark-frames "${OSGSPONZA_PROFILE_BENCHMARK_FRAMES}"
    --benchmark-warmup-frames "${OSGSPONZA_PROFILE_BENCHMARK_WARMUP_FRAMES}"
    ${_osgsponza_profile_extra_args})

find_program(OSGSPONZA_TIME_COMMAND time)
if(OSGSPONZA_TIME_COMMAND)
    add_custom_target(osgsponza_profile_time
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OSGSPONZA_PROFILE_DIR}"
        COMMAND ${OSGSPONZA_TIME_COMMAND} -v
            -o "${OSGSPONZA_PROFILE_DIR}/time-camera-${OSGSPONZA_PROFILE_CAMERA_INDEX}.txt"
            ${_osgsponza_profile_command}
        DEPENDS example_osgsponza
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "osgsponza: timing camera ${OSGSPONZA_PROFILE_CAMERA_INDEX}"
        VERBATIM
    )
endif()

if(ENABLE_PERF)
    add_custom_target(osgsponza_perf_stat
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OSGSPONZA_PROFILE_DIR}"
        COMMAND ${PERF} stat -d
            -o "${OSGSPONZA_PROFILE_DIR}/perf-stat-camera-${OSGSPONZA_PROFILE_CAMERA_INDEX}.txt"
            -- ${_osgsponza_profile_command}
        DEPENDS example_osgsponza
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "osgsponza: perf stat camera ${OSGSPONZA_PROFILE_CAMERA_INDEX}"
        VERBATIM
    )

    add_custom_target(osgsponza_perf_record
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OSGSPONZA_PROFILE_DIR}"
        COMMAND ${PERF} record -F 99 -g --call-graph dwarf
            -o "${OSGSPONZA_PROFILE_DIR}/perf-camera-${OSGSPONZA_PROFILE_CAMERA_INDEX}.data"
            -- ${_osgsponza_profile_command}
        DEPENDS example_osgsponza
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "osgsponza: perf record camera ${OSGSPONZA_PROFILE_CAMERA_INDEX}"
        VERBATIM
    )

    add_custom_target(osgsponza_perf_report
        COMMAND sh -c
            "${PERF} report --stdio -i '${OSGSPONZA_PROFILE_DIR}/perf-camera-${OSGSPONZA_PROFILE_CAMERA_INDEX}.data' > '${OSGSPONZA_PROFILE_DIR}/perf-report-camera-${OSGSPONZA_PROFILE_CAMERA_INDEX}.txt'"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "osgsponza: perf report camera ${OSGSPONZA_PROFILE_CAMERA_INDEX}"
        VERBATIM
    )
endif()
