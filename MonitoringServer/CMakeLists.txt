cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME Profiler)
set(CMAKE_CXX_STANDARD 17)

# 실행 파일은 빌드 디렉토리 안에 bin 폴더에 출력.
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(WINDOW_NAME "Profiler")
set(WINDOW_WIDTH 1280)
set(WINDOW_HEIGHT 720)

project(${PROJECT_NAME})

set(CMAKE_VERBOSE_MAKEFILE true)

add_executable(${PROJECT_NAME}   
    src/main.cpp
    src/Enum.pb.h src/Enum.pb.cc
    src/Protocol.pb.h src/Protocol.pb.cc
    src/Struct.pb.h src/Struct.pb.cc
    src/ServerPacketHandler.h src/ServerPacketHandler.cpp
    src/MonitoringServer.h src/MonitoringServer.cpp
    src/ServerSession.h src/ServerSession.cpp
)

include(Dependency.cmake)

# PUBLIC -> 해당 target에 의존성을 가지는 또 다른 타겟
# Ex) target이 라이브러리이고, 이 라이브러리를 링크하는 어플리케이션에도 동일한 정의가 적용
# PRIVATE -> 해당 target에만 적용

# 우리 프로젝트에 include / lib 관련 옵션 추가
# DEP_INCLUDE_DIR -> build/include
# DEP_LIB_DIR -> build/lib
# DEP_LIBS -> spdlogd(Debug) glfw3
target_include_directories(${PROJECT_NAME} PUBLIC ${DEP_INCLUDE_DIR})
target_link_directories(${PROJECT_NAME} PUBLIC ${DEP_LIB_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${DEP_LIBS})

# target_compile_definitions() -> 특정 타겟에 대한 컴파일 정의 ( #define )
target_compile_definitions(${PROJECT_NAME} PUBLIC
    WINDOW_NAME="${WINDOW_NAME}"
    WINDOW_WIDTH=${WINDOW_WIDTH}
    WINDOW_HEIGHT=${WINDOW_HEIGHT}
)

# Dependency들이 먼저 build 될 수 있게 관계 설정
add_dependencies(${PROJECT_NAME} ${DEP_LIST})


# CMake Config -> Build
# cmake -Bbuild . -DCMAKE_BUILD_TYPE=Debug|Release
# cmake --build build