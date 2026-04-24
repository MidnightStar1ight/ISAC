cmake_minimum_required(VERSION 3.16)

project(isac4 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Путь к собранной библиотеке ISAC
set(ISAC_LIB_PATH "${CMAKE_SOURCE_DIR}/lib")
set(ISAC_INCLUDE_PATH "${CMAKE_SOURCE_DIR}")
set(ISAC_H_PATH "${CMAKE_SOURCE_DIR}/webrtc/modules/audio_coding/codecs/isac/main/interface")

# Добавляем заголовочные файлы
include_directories(${ISAC_INCLUDE_PATH})
# include_directories(${ISAC_H_PATH})



# Добавляем библиотеку
add_library(isac_codec STATIC IMPORTED)
set_target_properties(isac_codec PROPERTIES
    # IMPORTED_LOCATION ${ISAC_LIB_PATH}/isac.lib
    IMPORTED_LOCATION ${ISAC_LIB_PATH}/libisac.a
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(isac4
  main.cpp
  application.h application.cpp
  isaccodec.h isaccodec.cpp
  audiohandler.h audiohandler.cpp
)

target_include_directories(isac4 PRIVATE ${ISAC_INCLUDE_PATH})
# target_include_directories(isac4 PRIVATE ${ISAC_H_PATH})

# Линкуем с ISAC
target_link_libraries(isac4 isac_codec)

# add_library(webrtc STATIC IMPORTED)
# set_target_properties(webrtc PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/libwebrtc/release/webrtc.lib")
# target_include_directories(isac4 PRIVATE "${CMAKE_SOURCE_DIR}/libwebrtc/include")
# target_link_libraries(isac4 webrtc)

target_link_libraries(isac4 Qt${QT_VERSION_MAJOR}::Core)

include(GNUInstallDirs)
install(TARGETS isac4
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
