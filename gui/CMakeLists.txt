add_executable(elf-gui main.cpp
        main-window.cpp main-window.h
        elf-header-model.cpp elf-header-model.h
)
target_include_directories(elf-gui PUBLIC ${QT5_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/macros)
target_link_libraries(elf-gui PUBLIC ${QT5_LIBRARIES})
target_compile_options(elf-gui PUBLIC -fPIC)
