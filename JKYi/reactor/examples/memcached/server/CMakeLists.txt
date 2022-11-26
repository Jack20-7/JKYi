add_executable(memcached Item.cc MemcacheServer.cc Session.cc server.cc)
target_link_libraries(memcached ${LIBS})
