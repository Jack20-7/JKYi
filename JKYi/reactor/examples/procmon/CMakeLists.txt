add_executable(procmon  plot.cc procmon.cc)
target_link_libraries(procmon ${LIBS} gd)

add_executable(dummyload dummyload.cc)
target_link_libraries(dummyload ${LIBS})
