file(GLOB CCOS_SRC
        "*.h"
        "*.cpp"
        "*.c"
        )

file(STRINGS "${CMAKE_SOURCE_DIR}/main/build-number.txt" CCOS_BUILD)

set(COMPONENT_SRCS ${CCOS_SRC})
set(COMPONENT_ADD_INCLUDEDIRS ".")

# Embed the server root certificate into the final binary
#set(COMPONENT_EMBED_TXTFILES ${PROJECT_PATH}/server_certs/ca_cert.pem)

register_component()
