add_executable(InnoTest InnoTest.cpp)
target_link_libraries(InnoTest InnoCore)

if (INNO_PLATFORM_LINUX)
target_link_libraries(InnoTest -lX11 -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor -ldl -lm -lpthread -lstdc++fs)
endif (INNO_PLATFORM_LINUX)
