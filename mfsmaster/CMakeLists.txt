include_directories(${CMAKE_CURRENT_SOURCE_DIR} ../mfscommon)
AUX_SOURCE_DIRECTORY("." MASTER_SRCS)
set(MAIN_SRC "../mfscommon/main.cc")

add_definitions(-DAPPNAME=mfsmaster)  # used for config file paths, locks, messages

add_executable(mfsmaster ${MASTER_SRCS} ${MAIN_SRC}) 
target_link_libraries(mfsmaster common ${ZLIB_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
