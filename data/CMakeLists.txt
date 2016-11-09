set(DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/data")

foreach(NAME images lyrics streams shaders)
    # can't use directory that will be modified as target, thus .phony
    add_custom_command(
        OUTPUT ${DATA_DIR}/${NAME}/.phony
        COMMAND mkdir -p ${DATA_DIR}
        COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/${NAME} ${DATA_DIR}/${NAME}
        COMMAND touch ${DATA_DIR}/${NAME}/.phony)
endforeach()

set(DUMPGLYPHS ${CMAKE_BINARY_DIR}/dumpglyphs/dumpglyphs)
set(FONT ${CMAKE_CURRENT_SOURCE_DIR}/font.ttf)

set(IMAGE_DIR ${DATA_DIR}/images)
set(FONT_DIR ${DATA_DIR}/fonts)

macro(gen_font NAME WIDTH HEIGHT SIZE GLYPHS)
    add_custom_command(
        OUTPUT ${FONT_DIR}/${NAME}_font.fnt ${IMAGE_DIR}/${NAME}_font.png
        COMMAND mkdir -p ${FONT_DIR}
        COMMAND ${DUMPGLYPHS} -b ffffff -W ${WIDTH} -H ${HEIGHT} -S ${SIZE} -I ${NAME} ${FONT} ${GLYPHS}
        COMMAND echo -n data/images/ | cat - ${NAME}_font.fnt > ${FONT_DIR}/${NAME}_font.fnt
        COMMAND mv ${NAME}_font.png ${IMAGE_DIR}/${NAME}_font.png
        DEPENDS ${DUMPGLYPHS} ${IMAGE_DIR}/.phony)
endmacro()

execute_process(
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/find_glyphs.pl
    OUTPUT_VARIABLE GLYPH_LIST
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

gen_font(tiny 1024 512 10 "${GLYPH_LIST}")
gen_font(small 1024 1024 18 "${GLYPH_LIST}")
gen_font(medium 1024 1024 24 "${GLYPH_LIST}")
gen_font(big_az 256 256 36 "65-90;48-57;45")

add_custom_target(
    genassets ALL
    DEPENDS
        ${DATA_DIR}/lyrics/.phony
        ${DATA_DIR}/images/.phony
        ${DATA_DIR}/streams/.phony
        ${DATA_DIR}/shaders/.phony
        ${FONT_DIR}/tiny_font.fnt ${IMAGE_DIR}/tiny_font.png
        ${FONT_DIR}/small_font.fnt ${IMAGE_DIR}/small_font.png
        ${FONT_DIR}/medium_font.fnt ${IMAGE_DIR}/medium_font.png
        ${FONT_DIR}/big_az_font.fnt ${IMAGE_DIR}/big_az_font.png)
