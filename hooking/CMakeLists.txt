set(gammaray_probe_srcs
  probecreator.cpp
  hooks.cpp
)

if(UNIX AND Qt5Core_VERSION VERSION_LESS 5.4.0)
  list(APPEND gammaray_probe_srcs preload.cpp)
endif()

if(WIN32 OR APPLE)
  list(APPEND gammaray_probe_srcs
    abstractfunctionoverwriter.cpp
    functionoverwriterfactory.cpp
    unixfunctionoverwriter.cpp
  )
endif()

if(WIN32)
  list(APPEND gammaray_probe_srcs
    entry_win.cpp
    winfunctionoverwriter.cpp
  )
endif()

if(APPLE OR NOT Qt5Core_VERSION VERSION_LESS 5.4.0)
  list(APPEND gammaray_probe_srcs entry_unix.cpp)
endif()

# probe lib
if(APPLE) # DYLD_INSERT_LIBRARIES only works with real shared libraries, not modules
  add_library(gammaray_probe SHARED ${gammaray_probe_srcs})
  set_target_properties(gammaray_probe PROPERTIES INSTALL_RPATH "@loader_path/../../../Frameworks")
else()
  add_library(gammaray_probe MODULE ${gammaray_probe_srcs})
endif()
target_link_libraries(gammaray_probe
  gammaray_core
)

if(NOT WIN32 AND NOT QNXNTO)
  target_link_libraries(gammaray_probe dl)
endif()

set_target_properties(gammaray_probe PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${PROBE_PLUGIN_INSTALL_DIR}"
)
if(NOT ANDROID)
  set_target_properties(gammaray_probe PROPERTIES PREFIX "")
endif()

install(TARGETS gammaray_probe DESTINATION ${PROBE_PLUGIN_INSTALL_DIR})
