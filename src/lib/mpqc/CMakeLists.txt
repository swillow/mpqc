
# set(CC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/cc/integrals.cpp")
# #file(GLOB CC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/cc/*.cpp")
# add_library(mpqc/cc OBJECT
#   ${CC_SOURCES}
# )

add_subdirectory(math)
#add_subdirectory(test)


add_subdirectory(ci)

# # library headers
# file(GLOB_RECURSE headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hpp)
# foreach (_file ${headers})
#   get_filename_component(_path ${_file} PATH)
#   #message(STATUS "gonna installl ${_file} in include/mpqc/${_path}" )
#   install(FILES ${_file} DESTINATION include/mpqc/${_path})
# endforeach()
# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config.h DESTINATION include/mpqc)

install(FILES range.hpp DESTINATION include/mpqc)

if (MPQC_CI)
  if (MPQC_UNITTEST)
    add_executable(file_test file_test.cpp)
    target_link_libraries(file_test
      util
      ${HDF5_LIBRARIES}
      ${Boost_UNITTEST_LIBRARIES}
      ${Boost_LIBRARIES}
    )
    add_test(file file_test)
  endif()
endif()
