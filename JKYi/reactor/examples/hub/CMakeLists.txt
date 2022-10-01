add_executable(hub hub.cc codec.cc)
target_link_libraries(hub ${LIBS})

#这里将客户端编成一个库
add_library(JKYi_pubsub pubsub.cc codec.cc)
target_link_libraries(JKYi_pubsub ${LIBS})

add_executable(pub pub.cc)
target_link_libraries(pub JKYi_pubsub)

add_executable(sub sub.cc)
target_link_libraries(sub JKYi_pubsub)
