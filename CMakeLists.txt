cmake_minimum_required(VERSION 3.12)

project(Converter VERSION 1.0.0
    DESCRIPTION "Simple program for converting strings"
    HOMEPAGE_URL "https://tim-gromeyer.github.io/md_Converter.html"
    LANGUAGES CXX C)
set(PROJECT_CONTACT "sakul8826@gmail.com")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Build type not specified. Release is used.")
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Svg is nessesary for android
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools PrintSupport)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND NOT CMAKE_CROSSCOMPILING)
    set(LINUX TRUE)
endif()

if (EMSCRIPTEN)
    add_compile_definitions(NO_THREADING)
endif()

set(TS_FILES translations/Converter_de_DE.ts)
set(RC_FILES
    ressources/icons.qrc
    ressources/ressources.qrc
    translations/translations.qrc
)

# Include windows icon
if(WIN32)
    list(APPEND PROJECT_SOURCES ressources/logo/Icon.rc)
endif()

# Include 3rdparty libs and translations
include(3rdparty/CMakeLists.txt)
include(cmake/TranslationUtils.cmake)

# svg required for android
if (ANDROID)
    find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS Svg)
    if (Qt${QT_VERSION_MAJOR}Svg_FOUND)
        message(STATUS "Include Qt Svg library")
        list(APPEND 3rdparty
            Qt${QT_VERSION_MAJOR}::Svg
        )
    endif()
endif(ANDROID)

set(PROJECT_SOURCES
    ${RC_FILES}
    common.h
    src/common.cpp
    src/converter.cpp
    src/converter.h
    src/highlighter.cpp
    src/highlighter.h
    src/imagedialouge.cpp
    src/imagedialouge.h
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.h
    src/markdownparser.cpp
    src/markdownparser.h
    src/mimetype.cpp
    src/mimetype.h
    src/typeparser.cpp
    src/typeparser.h
    ui/mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_translation(QM_FILES ${TS_FILES})
    add_app_translations_resource(APP_RES ${QM_FILES})

    qt_add_executable(converter
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${APP_RES}
    )

    set_property(TARGET converter APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
                 ${CMAKE_CURRENT_SOURCE_DIR}/android)

else()
    qt5_add_translation(QM_FILES ${TS_FILES})
    add_app_translations_resource(APP_RES ${QM_FILES})

    if(ANDROID)
        add_library(converter SHARED
            ${PROJECT_SOURCES}
            ${APP_RES}
        )

        set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")

    else()
        add_executable(converter
            ${PROJECT_SOURCES}
            ${APP_RES}
        )
    endif()
endif()

IF(WIN32)
    SET(INTL_LDFLAGS -lintl)
ENDIF(WIN32)

target_include_directories(converter PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(converter PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::PrintSupport ${INTL_LDFLAGS} ${3rdparty})

target_compile_definitions(converter PUBLIC
    APP_VERSION="${PROJECT_VERSION}"
    QT_DISABLE_DEPRECATED_BEFORE=0x051208
    QT_NO_REGEXP
    QT_USE_QSTRINGBUILDER
)

set_target_properties(converter PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER org.qtproject.converter
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

set_target_properties(converter PROPERTIES OUTPUT_NAME "sconverter")

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(converter)
endif()

# Packaging
include(GNUInstallDirs)

find_package(ECM 5.86.0 QUIET NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
if(ECM_FOUND)
    include(ECMInstallIcons)
    ecm_install_icons(
        ICONS
            ressources/icon/16-apps-SConverter.png
            ressources/icon/24-apps-SConverter.png
            ressources/icon/32-apps-SConverter.png
            ressources/icon/48-apps-SConverter.png
            ressources/icon/64-apps-SConverter.png
            ressources/icon/128-apps-SConverter.png
            ressources/icon/256-apps-SConverter.png
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons
        THEME hicolor
    )
endif()

install(TARGETS converter DESTINATION ${CMAKE_INSTALL_BINDIR})

# Desktop shortcut for linux
if(LINUX)
    install(FILES packaging/SConverter.desktop
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
endif()

# Icon for windows
if(WIN32)
    install(FILES ressouces/icon/Icon.ico
            DESTINATION .)
endif()

include(cmake/Packaging.cmake)
