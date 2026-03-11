# ── Sanitizers ───────────────────────────────────────────────
# Default: ASan + UBSan.
# Switch:  -DTHREAD_SANITIZE=ON  →  TSan + UBSan  (disables ASan)
#          -DMEMORY_SANITIZE=ON  →  MSan + UBSan  (disables ASan)
# MSVC: ASan only (/fsanitize=address). No UBSan/TSan/MSan support.
# ─────────────────────────────────────────────────────────────

option(THREAD_SANITIZE "Use ThreadSanitizer instead of AddressSanitizer" OFF)
option(MEMORY_SANITIZE "Use MemorySanitizer instead of AddressSanitizer" OFF)

if(THREAD_SANITIZE AND MEMORY_SANITIZE)
    message(FATAL_ERROR
        "THREAD_SANITIZE and MEMORY_SANITIZE cannot both be ON.\n"
        "TSan and MSan use incompatible shadow memory layouts.")
endif()

if(MSVC)
    if(THREAD_SANITIZE OR MEMORY_SANITIZE)
        message(WARNING "MSVC does not support TSan or MSan. Using ASan only.")
    endif()
    add_compile_options(/fsanitize=address)
    message(STATUS "Sanitizers: ASan (MSVC)")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # UBSan is always on — compatible with all three
    set(SANITIZE_FLAGS "-fsanitize=undefined")

    if(THREAD_SANITIZE)
        string(APPEND SANITIZE_FLAGS ",thread")
        message(STATUS "Sanitizers: TSan + UBSan")
    elseif(MEMORY_SANITIZE)
        string(APPEND SANITIZE_FLAGS ",memory")
        message(STATUS "Sanitizers: MSan + UBSan")
    else()
        string(APPEND SANITIZE_FLAGS ",address")
        message(STATUS "Sanitizers: ASan + UBSan (default)")
    endif()

    add_compile_options(${SANITIZE_FLAGS} -fno-omit-frame-pointer)
    add_link_options(${SANITIZE_FLAGS})
else()
    message(STATUS "Sanitizers: skipped (unsupported compiler)")
endif()
