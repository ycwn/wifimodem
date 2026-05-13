

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.c)


idf_component_register(
	SRCS         ${app_sources}
	REQUIRES     mbedtls
	LDFRAGMENTS  "${CMAKE_SOURCE_DIR}/wifimodem.lf"
)


#
# Disable bullshit errors
#
idf_build_get_property(cflags  COMPILE_OPTIONS)

list(REMOVE_ITEM cflags -Werror -Werror=all -Woverride-init)

set_property(
	TARGET                    ${COMPONENT_TARGET}
	PROPERTY COMPILE_OPTIONS "${cflags}"
)

idf_build_set_property(LINK_OPTIONS   "-Wl,--Map=${BUILD_DIR}/wifimodem.map" APPEND)
idf_build_set_property(LINK_OPTIONS   "-Wl,--orphan-handling=place" APPEND)


