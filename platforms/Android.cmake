add_library(main SHARED ${RETRO_FILES})

set(DEP_PATH android)
set(RETRO_SDL_VERSION 2)
set(RETRO_USE_HW_RENDER OFF)
set(RETRO_OUTPUT_NAME "RetroEngine")

set(COMPILE_OGG TRUE)
set(COMPILE_THEORA TRUE)
set(COMPILE_VORBIS TRUE)

set(OGG_FLAGS -ffast-math -fsigned-char -O2 -fPIC -DPIC -DBYTE_ORDER=LITTLE_ENDIAN -D_ARM_ASSEM_ -w)
set(THEORA_FLAGS -ffast-math -fsigned-char -O2 -fPIC -DPIC -DBYTE_ORDER=LITTLE_ENDIAN -D_ARM_ASSEM_ -w)
set(VORBIS_FLAGS -ffast-math -fsigned-char -O2 -fPIC -DPIC -DBYTE_ORDER=LITTLE_ENDIAN -D_ARM_ASSEM_ -w)

target_link_libraries(RetroEngine
    android
    EGL
    GLESv1_CM
    log
    z
    jnigraphics
    openslES
    SDL2
)

target_link_options(main PRIVATE -u GameActivity_onCreate)
