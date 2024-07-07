cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# PICO_SDK_PATH is assumed to be set in the environment
#set(PICO_SDK_PATH "C:/Dev/Pico/pico-sdk")

include(pico_sdk_import.cmake)

set(PICO_SDK_VERSION_REQUIRED "1.4.0")
if (${PICO_SDK_VERSION_STRING} VERSION_LESS PICO_SDK_VERSION_REQUIRED)
    message(WARNING "Pico SDK version ${PICO_SDK_VERSION_REQUIRED}+ is required")
endif()

project(TaskDemo C CXX ASM)

# TODO: Set the type of the target dev board
#set(PICO_BOARD "pico")
set(PICO_BOARD "adafruit_feather_rp2040")

# Initialise the Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name.
add_executable(TaskDemo main.cpp)
pico_set_program_name(TaskDemo "TaskDemo")
#pico_set_program_version(Dexy "0.0") # No - this will be set by MakeVersionFile & Dexy

# Update version info file from the latest git tag
list(APPEND VERSION_FILES "${PROJECT_SOURCE_DIR}/version.h" "${PROJECT_SOURCE_DIR}/VERSION")
list(TRANSFORM VERSION_FILES APPEND ".template" OUTPUT_VARIABLE VERSION_TEMPLATES)
target_sources(TaskDemo PRIVATE ${VERSION_FILES})
set(CMD_PYTHON "py") # for Windows
#set(CMD_PYTHON "/usr/local/bin/python3.11") # for macOS, Linux
set(VERSION_TEMP_FILE "${PROJECT_BINARY_DIR}/version-temp")
set(VERSION_INFO_FILE "${PROJECT_BINARY_DIR}/version-info")
add_custom_target(MakeVersionFile
    COMMAND git describe --tags --always --dirty >${VERSION_TEMP_FILE}
    COMMAND ${CMD_PYTHON} ${PROJECT_SOURCE_DIR}/update-version-info.py
        ${VERSION_INFO_FILE}
        ${VERSION_TEMP_FILE}
)
add_dependencies(TaskDemo MakeVersionFile)
add_custom_command(
    OUTPUT ${VERSION_FILES}
    DEPENDS ${VERSION_INFO_FILE} ${VERSION_TEMPLATES}
    COMMAND ${CMD_PYTHON} ${PROJECT_SOURCE_DIR}/make-version-file.py
        ${VERSION_INFO_FILE}
        ${VERSION_FILES}
)

# Set compiler options
target_compile_options(TaskDemo PUBLIC "-Wall;-Wextra;-Wconversion;-Wshadow")
# Save compiler temp files. Assembler output will be in *.s.
#target_compile_options(TaskDemo PUBLIC "-save-temps")
# Hide warnings in Pico SDK files
set_source_files_properties(
    ${PICO_SDK_PATH}/src/rp2_common/hardware_flash/flash.c
    ${PICO_TINYUSB_PATH}/src/portable/raspberrypi/rp2040/rp2040_usb.c
    ${PICO_TINYUSB_PATH}/src/tusb.c
    ${PICO_TINYUSB_PATH}/src/common/tusb_fifo.c
    ${PICO_TINYUSB_PATH}/src/device/usbd.c
    ${PICO_TINYUSB_PATH}/src/device/usbd_control.c
    ${PICO_TINYUSB_PATH}/src/class/cdc/cdc_device.c
    PROPERTIES
    COMPILE_FLAGS "-Wno-array-bounds -Wno-stringop-overflow -Wno-conversion -Wno-shadow")
target_link_options(TaskDemo PRIVATE "LINKER:--no-warn-rwx-segments")

# TODO: Enable stdio via UART and/or USB
#pico_enable_stdio_uart(TaskDemo 1)
pico_enable_stdio_usb(TaskDemo 1)

# Default stage-2 boot file (despite what it says in the Adafruit Feather board file)
set(PICO_DEFAULT_BOOT_STAGE2_FILE "${CMAKE_CURRENT_LIST_DIR}/boot2_w25qs080.S")

# TODO: Add link libraries
target_link_libraries(TaskDemo pico_stdlib hardware_pio)

# Run executable from RAM instead of flash
#pico_set_binary_type(TaskDemo copy_to_ram)

# Create a .uf2 executable file
pico_add_extra_outputs(TaskDemo)
