cmake_minimum_required(VERSION 3.16)

project(QtClientSSN VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ✅ Multimedia 모듈 추가
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia MultimediaWidgets)

qt_standard_project_setup()

set(SOURCES
    main.cpp
    mainwindow.cpp
    loginwindow.cpp
    cameraregistrationdialog.cpp
    loghistorydialog.cpp
)

set(HEADERS
    mainwindow.h
    loginwindow.h
    cameraregistrationdialog.h
    loghistorydialog.h
)

set(FORMS
    mainwindow.ui
)

qt_add_executable(QtClientSSN
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
)

# ✅ Multimedia 링크 추가
target_link_libraries(QtClientSSN
    PRIVATE Qt6::Core
    PRIVATE Qt6::Widgets
    PRIVATE Qt6::Multimedia
    PRIVATE Qt6::MultimediaWidgets
)

# Windows 전용 속성 (아이콘 없는 윈도우 실행)
if(WIN32)
    set_target_properties(QtClientSSN PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "QtClientSSN_Camera_System"
    )
endif()
