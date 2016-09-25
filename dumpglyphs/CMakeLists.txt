find_package(Freetype REQUIRED)
find_package(PNG REQUIRED)

include_directories(
    ${FREETYPE_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS})

add_executable(dumpglyphs dumpglyphs.c)

target_link_libraries(
    dumpglyphs
    ${FREETYPE_LIBRARIES}
    ${PNG_LIBRARIES})
