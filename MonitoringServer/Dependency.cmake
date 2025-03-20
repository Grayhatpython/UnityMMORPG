
include(ExternalProject)

# PROJECT_BINARY_DIR - > build Folder
# DEP_INCLUDE_DIR -> build/external/include
# DEP_LIB_DIR -> build/external/lib
set(DEP_EXTERNAL_DIR ${PROJECT_BINARY_DIR}/external)
set(DEP_INCLUDE_DIR ${DEP_EXTERNAL_DIR}/include)
set(DEP_LIB_DIR ${DEP_EXTERNAL_DIR}/lib)

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC OpenGL::GL)
target_link_libraries(${PROJECT_NAME} PUBLIC odbc32)
# glfw
ExternalProject_Add(
    dep-glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG "3.3.4"
    GIT_SHALLOW 1
    UPDATE_COMMAND ""
    PATCH_COMMAND "" 
    TEST_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${DEP_EXTERNAL_DIR}
        #   GLFW CMakeLists.txt -> option
        -DGLFW_BUILD_EXAMPLES=OFF   # 예제 x
        -DGLFW_BUILD_TESTS=OFF      # 테스트 x
        -DGLFW_BUILD_DOCS=OFF       # 문서 x
    )

set(DEP_LIST ${DEP_LIST} dep-glfw)
set(DEP_LIBS ${DEP_LIBS} glfw3)

# glad
ExternalProject_Add(
    dep-glad
    GIT_REPOSITORY "https://github.com/Dav1dde/glad"
    GIT_TAG "v0.1.36"
    GIT_SHALLOW 1
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    TEST_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${DEP_EXTERNAL_DIR}
        -DGLAD_INSTALL=ON
    )

set(DEP_LIST ${DEP_LIST} dep-glad)
set(DEP_LIBS ${DEP_LIBS} glad)

# imgui
# target name 
# No Update So Only Cpp file 
add_library(imgui
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/imgui_impl_glfw.cpp
    imgui/imgui_impl_opengl3.cpp
    imgui/imgui_demo.cpp
    )

target_include_directories(imgui PRIVATE ${DEP_INCLUDE_DIR})
add_dependencies(imgui ${DEP_LIST})

set(DEP_INCLUDE_DIR ${DEP_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

set(DEP_LIST ${DEP_LIST} imgui)
set(DEP_LIBS ${DEP_LIBS} imgui)

# implot
add_library(implot
    implot/implot_items.cpp
    implot/implot.cpp
    )

set(DEP_INCLUDE_DIR ${DEP_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/implot)

set(DEP_LIST ${DEP_LIST} implot)
set(DEP_LIBS ${DEP_LIBS} implot)

target_include_directories(implot PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
add_dependencies(implot imgui)

target_link_libraries(implot PRIVATE imgui)

# protobuf
set(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(PROTOBUF_INCLUDE_DIR ${PARENT_DIR}/Libraries/Include)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(PROTOBUF_LIB_DIR ${PARENT_DIR}/Libraries/Libs/Protobuf/Debug)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(PROTOBUF_LIB_DIR ${PARENT_DIR}/Libraries/Libs/Protobuf/Release)
else()
    set(PROTOBUF_LIB_DIR ${PARENT_DIR}/Libraries/Libs/Protobuf/$<CONFIG>)
endif()

set(DEP_INCLUDE_DIR ${DEP_INCLUDE_DIR} ${PROTOBUF_INCLUDE_DIR})
set(DEP_LIB_DIR ${DEP_LIB_DIR} ${PROTOBUF_LIB_DIR})
set(DEP_LIBS ${DEP_LIBS} libprotobuf$<$<CONFIG:Debug>:d>)

# server Core
set(SERVER_CORE_INCLUDE_DIR ${PARENT_DIR}/ServerCore)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SERVER_CORE_LIB_DIR ${PARENT_DIR}/Libraries/Libs/ServerCore/Debug)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(SERVER_CORE_LIB_DIR ${PARENT_DIR}/Libraries/Libs/ServerCore/Release)
else()
    set(SERVER_CORE_LIB_DIR ${PARENT_DIR}/Libraries/Libs/ServerCore/$<CONFIG>)
endif()

set(DEP_INCLUDE_DIR ${DEP_INCLUDE_DIR} ${SERVER_CORE_INCLUDE_DIR})
set(DEP_LIB_DIR ${DEP_LIB_DIR} ${SERVER_CORE_LIB_DIR})
set(DEP_LIBS ${DEP_LIBS} ServerCore)