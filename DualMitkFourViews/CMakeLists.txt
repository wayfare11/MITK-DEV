cmake_minimum_required(VERSION 3.18)

project(DualMitkFourViews LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

if(NOT DEFINED MITK_DIR AND DEFINED ENV{MITK_DIR})
  file(TO_CMAKE_PATH "$ENV{MITK_DIR}" MITK_DIR)
  set(MITK_DIR "${MITK_DIR}" CACHE PATH "MITK build directory")
endif()

if(NOT DEFINED Qt5_DIR AND EXISTS "D:/ProgramData/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5")
  set(Qt5_DIR "D:/ProgramData/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5" CACHE PATH "Qt5 CMake directory")
endif()

find_package(MITK REQUIRED)
find_package(ITK REQUIRED COMPONENTS ITKIONRRD)
find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(CPP_FILES
  src/main.cpp
)

usFunctionGenerateModuleInit(CPP_FILES)

add_executable(DualMitkFourViews WIN32
  ${CPP_FILES}
)

target_link_libraries(DualMitkFourViews PRIVATE
  MitkCore
  MitkQtWidgets
  MitkQtWidgetsExt
  ITKIONRRD
  Qt5::Widgets
)

target_compile_definitions(DualMitkFourViews PRIVATE US_MODULE_NAME=main)
set_property(TARGET DualMitkFourViews PROPERTY US_MODULE_NAME main)

if(WIN32 AND DEFINED MITK_CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  add_custom_command(TARGET DualMitkFourViews POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${MITK_CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MitkCore/$<CONFIG>"
      "${CMAKE_BINARY_DIR}/MitkCore/$<CONFIG>"
    COMMENT "Copy MITK Core auto-load modules for standalone execution")
endif()

if(MSVC)
  target_compile_options(DualMitkFourViews PRIVATE /utf-8)
  target_compile_definitions(DualMitkFourViews PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
  set_property(TARGET DualMitkFourViews PROPERTY
    VS_DEBUGGER_ENVIRONMENT "PATH=%MITK_2023_RELEASE%;%PATH%")
endif()
