add_library(rc_e2sm_objects
  rc_subscription_handler.cpp
)

target_include_directories(rc_e2sm_objects PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
