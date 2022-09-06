set(srcs main.cpp ../firmware_target/firmware_target.c)
set(include_dirs . ../firmware_target)

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS ${include_dirs} "."
                       REQUIRES esp-serial-flasher u8g2 u8g2 esp-u8g2-hal esp32-button)

set(target ${COMPONENT_LIB})

# Embed binaries into the app.
# In ESP-IDF this can also be done using EMBED_FILES option of idf_component_register.
# Here an external tool is used to make file embedding similar with other ports.
include(${CMAKE_CURRENT_LIST_DIR}/../firmware_target/bin2array.cmake)
create_resources(${CMAKE_CURRENT_LIST_DIR}/../firmware_target/bin ${CMAKE_BINARY_DIR}/binaries.c)
set_property(SOURCE ${CMAKE_BINARY_DIR}/binaries.c PROPERTY GENERATED 1)
target_sources(${target} PRIVATE ${CMAKE_BINARY_DIR}/binaries.c)
