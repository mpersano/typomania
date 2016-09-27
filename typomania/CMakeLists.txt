set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(SDL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OggVorbis REQUIRED)
find_package(OpenAL REQUIRED)
find_package(PNG REQUIRED)

include_directories(
    ${SDL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${VORBIS_INCLUDE_DIR}
    ${OGG_INCLUDE_DIR}
    ${OPENAL_INCLUDE_DIR}
    ${PNG_INCLUDE_DIRS})

set(TYPOMANIA_SOURCES
    fft.cc
    font.cc
    game.cc
    gl_texture.cc
    gl_vertex_array.cc
    glyph_fx.cc
    image.cc
    in_game_state.cc
    kana.cc
    kashi.cc
    main.cc
    ogg_player.cc
    panic.cc
    pattern.cc
    song_menu_state.cc
    spectrum_bars.cc
    utf8.cc)

add_executable(typomania ${TYPOMANIA_SOURCES})

target_link_libraries(
    typomania
    ${SDL_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${VORBIS_LIBRARY}
    ${OGG_LIBRARY}
    ${VORBISFILE_LIBRARY}
    ${OPENAL_LIBRARY}
    ${PNG_LIBRARIES})

set(DATA_DIR "${CMAKE_BINARY_DIR}/data/data")

add_custom_command(TARGET typomania POST_BUILD
    COMMAND ln -sf ${DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR}/data
    DEPENDS ${DATA_DIR})
