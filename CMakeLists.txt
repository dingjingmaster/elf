cmake_minimum_required(VERSION 3.31)
project(elf LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(
        -w -g
        -D QT_NO_KEYWORDS
        -D QT_MESSAGELOGCONTEXT
)

find_package (PkgConfig)
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

pkg_check_modules(QT5 REQUIRED Qt5Core Qt5Widgets)


cmake_host_system_information(RESULT OS QUERY OS_NAME)
cmake_host_system_information(RESULT RELEASE QUERY OS_RELEASE)
execute_process(COMMAND uname -m OUTPUT_VARIABLE OS_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND bash -c "cat /etc/os-release|grep -E '^NAME'|awk -F'=' '{print $2}'|sed 's/\"//g'|tr '\n' ' '|sed 's/ //g'" OUTPUT_VARIABLE DISTRIB)

if ("${DISTRIB}" STREQUAL "Gentoo")
    execute_process(COMMAND bash -c "/lib64/qt5/bin/qmake --version | grep -i 'qt version' | awk '{print $4}' | tr '\n' ' '|sed 's/ //g'" OUTPUT_VARIABLE CUR_QT_VERSION)
else ()
    execute_process(COMMAND bash -c "qmake --version | grep -i 'qt version' | awk '{print $4}' | tr '\n' ' '|sed 's/ //g'" OUTPUT_VARIABLE CUR_QT_VERSION)
endif ()

MESSAGE("")
MESSAGE(STATUS "OS name         : '${OS}'")
MESSAGE(STATUS "OS arch         : '${OS_ARCH}'")
MESSAGE(STATUS "OS distribute   : '${DISTRIB}'")
MESSAGE(STATUS "OS release      : '${RELEASE}'")
MESSAGE(STATUS "Qt Version      : '${CUR_QT_VERSION}'")
MESSAGE("")

add_subdirectory(cmdline)
add_subdirectory(gui)
