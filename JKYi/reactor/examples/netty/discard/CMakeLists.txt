add_executable(netty_discard_client client.cc)
target_link_libraries(netty_discard_client ${LIBS})

add_executable(netty_discard_server server.cc)
target_link_libraries(netty_discard_server ${LIBS})
