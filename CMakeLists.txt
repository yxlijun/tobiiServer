project(tobiiServer)
cmake_minimum_required(VERSION 2.8)
find_package(OpenCV REQUIRED)
INCLUDE_DIRECTORIES("E:/code/tobiiProject/ZeroMQ 4.0.4/include")
LINK_DIRECTORIES("E:/code/tobiiProject/ZeroMQ 4.0.4/lib")
INCLUDE_DIRECTORIES("E:/code/tobiiProject/zguide/examples/C")
INCLUDE_DIRECTORIES("E:/code/tobiiProject/tobii-eyex-sdk/include/eyex")
LINK_DIRECTORIES("E:/code/tobiiProject/tobii-eyex-sdk/lib/x86")

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
file(GLOB INC include/*.h include/*.hpp)
file(GLOB SRC src/*.c src/*.cc src/*.cpp)

source_group(include FILES ${INC})
source_group(src FILES ${SRC})

add_definitions( -DQLEMR_CFG_DLL )  
ADD_DEFINITIONS(-D_AFXDLL)  
SET(CMAKE_MFC_FLAG 2)

add_executable(${PROJECT_NAME} ${INC} ${SRC})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} libzmq-v120-mt-gd-4_0_4 )