# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "CMakeFiles\\nexgen_tray_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\nexgen_tray_autogen.dir\\ParseCache.txt"
  "external\\nexgen_qt_sys\\CMakeFiles\\nexgen_qt_sys_autogen.dir\\AutogenUsed.txt"
  "external\\nexgen_qt_sys\\CMakeFiles\\nexgen_qt_sys_autogen.dir\\ParseCache.txt"
  "external\\nexgen_qt_sys\\nexgen_qt_sys_autogen"
  "external\\nexgen_qt_themeset\\CMakeFiles\\nexgen_qt_themeset_autogen.dir\\AutogenUsed.txt"
  "external\\nexgen_qt_themeset\\CMakeFiles\\nexgen_qt_themeset_autogen.dir\\ParseCache.txt"
  "external\\nexgen_qt_themeset\\nexgen_qt_themeset_autogen"
  "nexgen_tray_autogen"
  )
endif()
