add_executable(server server_efficient.cc)
target_link_libraries(server ${LIBS})

add_executable(client client.cc)
target_link_libraries(client ${LIBS})

add_executable(loadtest loadtest.cc)
target_link_libraries(loadtest ${LIBS})
