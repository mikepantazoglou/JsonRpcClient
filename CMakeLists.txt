cmake_minimum_required(VERSION 2.8)

project(JsonRpcClient)

set(TARGET_NAME JsonRpcClient)
set(LIBWEBSOCKETS_VERSION 1.6.2)

set(SOURCE_FILES
  src/sonRpcClient.cpp
  src/SynchronousBlockingQueue.cpp
  src/WebSocketClient.cpp
  src/WebSocket.cpp
  src/WebSocketMessageHandler.cpp
)

set(SOURCE_FILES
  src/JsonRpcClient.h
  src/SynchronousBlockingQueue.h
  src/WebSocketClient.h
  src/WebSocket.h
  src/WebSocketMessageHandler.h
)

include_directories(
  dep/json
  dep/libwebsockets-${LIBWEBSOCKETS_VERSION}
  dep/libwebsockets-${LIBWEBSOCKETS_VERSION}/lib
  src
)

add_library(${TARGET_NAME} SHARED
  ${SOURCE_FILES}
  ${HEADER_FILES}
)

add_subdirectory(dep/libwebsockets-${LIBWEBSOCKETS_VERSION})
add_dependencies(${TARGET_NAME} websockets)
target_link_libraries(${TARGET_NAME} PRIVATE websockets)

set_target_properties(${TARGET_NAME} PROPERTIES 
  LINKER_LANGUAGE CXX
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED YES
)

