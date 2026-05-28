#-----------------------------------------------------------------------------
# TBB
#-----------------------------------------------------------------------------
set(proj_TBB TBB)
set(TBB_INSTALL_DIR ${ep_prefix}/TBB-install)

ExternalProject_Add(${proj_TBB}
  GIT_REPOSITORY https://github.com/oneapi-src/oneTBB.git
  GIT_TAG v2021.9.0
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=${TBB_INSTALL_DIR}
    -DTBB_TEST=OFF
    -DTBB_STRICT=OFF
)

#-----------------------------------------------------------------------------
# VTK
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED VTK_DIR AND NOT EXISTS ${VTK_DIR})
  message(FATAL_ERROR "VTK_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj VTK)
set(proj_DEPENDENCIES TBB)
set(VTK_DEPENDS ${proj})

if(NOT DEFINED VTK_DIR)

  set(additional_cmake_args )

  if(WIN32)
    list(APPEND additional_cmake_args
      -DCMAKE_CXX_MP_FLAG:BOOL=ON
      )
  else()
    list(APPEND additional_cmake_args
      -DVTK_MODULE_USE_EXTERNAL_VTK_freetype:BOOL=ON
      )
  endif()

  option(MITK_VTK_DEBUG_LEAKS OFF)
  mark_as_advanced(MITK_VTK_DEBUG_LEAKS)
  list(APPEND additional_cmake_args
    -DVTK_DEBUG_LEAKS:BOOL=${MITK_VTK_DEBUG_LEAKS}
    )

  if(MITK_USE_Qt5)
    list(APPEND additional_cmake_args
      -DVTK_GROUP_ENABLE_Qt:STRING=YES
      -DQt5_DIR:PATH=${Qt5_DIR}
      )
  endif()

  if(CTEST_USE_LAUNCHERS)
    list(APPEND additional_cmake_args
      "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
  endif()

  mitk_query_custom_ep_vars()

  ExternalProject_Add(${proj}
    LIST_SEPARATOR ${sep}
    GIT_REPOSITORY https://github.com/Kitware/VTK.git
    GIT_TAG v9.2.6
    GIT_SUBMODULES ""
    CMAKE_GENERATOR ${gen}
    CMAKE_GENERATOR_PLATFORM ${gen_platform}
    CMAKE_ARGS
      ${ep_common_args}
      -DOpenGL_GL_PREFERENCE:STRING=LEGACY
      -DVTK_ENABLE_WRAPPING:BOOL=OFF
      -DVTK_LEGACY_REMOVE:BOOL=ON
      -DVTK_MODULE_ENABLE_VTK_TestingRendering:STRING=YES
      -DVTK_MODULE_ENABLE_VTK_RenderingContextOpenGL2:STRING=YES
      -DVTK_MODULE_ENABLE_VTK_RenderingVolumeOpenGL2:STRING=YES
      -DVTK_MODULE_ENABLE_VTK_GUISupportQtQuick:STRING=NO
      -DVTK_MODULE_ENABLE_VTK_IOIOSS:STRING=NO # See T29633
      -DVTK_MODULE_ENABLE_VTK_ioss:STRING=NO   # See T29633
      -DVTK_QT_VERSION:STRING=5
      -DVTK_SMP_IMPLEMENTATION_TYPE:STRING=TBB
      -DTBB_DIR:PATH=${TBB_INSTALL_DIR}/lib/cmake/TBB
      ${additional_cmake_args}
      ${${proj}_CUSTOM_CMAKE_ARGS}
    CMAKE_CACHE_ARGS
      ${ep_common_cache_args}
      ${${proj}_CUSTOM_CMAKE_CACHE_ARGS}
    CMAKE_CACHE_DEFAULT_ARGS
      ${ep_common_cache_default_args}
      ${${proj}_CUSTOM_CMAKE_CACHE_DEFAULT_ARGS}
    DEPENDS ${proj_DEPENDENCIES}
    )

  set(VTK_DIR ${ep_prefix})
  mitkFunctionInstallExternalCMakeProject(${proj})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
