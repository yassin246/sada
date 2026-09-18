# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles/sadaudio_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/sadaudio_autogen.dir/ParseCache.txt"
  "sadaudio_autogen"
  )
endif()
