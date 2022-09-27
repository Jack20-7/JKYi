add_executable(idleconnection_echo echo.cc main.cc)
target_link_libraries(idleconnection_echo ${LIBS})

add_executable(idleconnection_echo2 sortedList.cc)
target_link_libraries(idleconnection_echo2 ${LIBS})
