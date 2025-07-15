cmake_minimum_required(VERSION 3.10)
project(LiveWallpaperApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(LiveWallpaperApp
    src/main.cpp
)

# Force the Windows API to Unicode
target_compile_definitions(LiveWallpaperApp PRIVATE UNICODE _UNICODE)

# Always link common Windows API libraries
target_link_libraries(LiveWallpaperApp PRIVATE
    user32
    gdi32
    ole32
    uuid
)

# On MinGW/GCC, link mingw32 and set -municode for wWinMain support
if (MSVC)
  set_target_properties(LiveWallpaperApp PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:CONSOLE"
  )
endif()
