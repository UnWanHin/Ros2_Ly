cmake_minimum_required(VERSION 3.16)
project(shooting_table_calib)

# 1. 基礎設置
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. 尋找 ROS 2 依賴
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(auto_aim_common REQUIRED)
find_package(tracker_solver REQUIRED)
find_package(predictor REQUIRED)
find_package(detector REQUIRED)

# 3. 尋找系統依賴
find_package(fmt REQUIRED) 
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(OpenVINO REQUIRED COMPONENTS Runtime)

# ========================================================
# [關鍵修復] 定義大恆相機庫
# 經過驗證，libgxiapi.so 已包含所有 Gx 和 Dx 函數
# 我們只需要定義這一個實體存在的庫即可
# ========================================================
add_library(GxIAPI SHARED IMPORTED GLOBAL)
set_property(TARGET GxIAPI PROPERTY IMPORTED_LOCATION "/usr/lib/libgxiapi.so")

# 刪除了對不存在的 /usr/lib/libDxImageProc.so 的引用
# 這樣 gmake 就不會因為找不到檔案而報錯
# ========================================================

# 4. 編譯庫 (Library)
file(GLOB_RECURSE LIB_SOURCES src/*.cpp)

if(LIB_SOURCES)
    add_library(${PROJECT_NAME} SHARED
        ${LIB_SOURCES}
    )

    target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${OpenCV_INCLUDE_DIRS}
        ${EIGEN3_INCLUDE_DIRS}
    )

    target_link_libraries(${PROJECT_NAME}
        fmt::fmt
        ${OpenCV_LIBS}
        Eigen3::Eigen
        openvino::runtime
        # [修復] 僅連結主庫，它已提供所有相機功能
        GxIAPI
    )

    ament_target_dependencies(${PROJECT_NAME}
        rclcpp
        std_msgs
        auto_aim_common
        tracker_solver
        predictor
        detector
    )
endif()

# 5. 編譯節點 (Node)
add_executable(${PROJECT_NAME}_node
    shooting_table_node.cpp
)

target_include_directories(${PROJECT_NAME}_node PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${OpenCV_INCLUDE_DIRS}
)

if(TARGET ${PROJECT_NAME})
    target_link_libraries(${PROJECT_NAME}_node
        ${PROJECT_NAME}
        openvino::runtime
        # [修復] 僅連結 GxIAPI
        GxIAPI
    )
else()
    target_link_libraries(${PROJECT_NAME}_node
        fmt::fmt
        ${OpenCV_LIBS}
        Eigen3::Eigen
        openvino::runtime
        # [修復] 僅連結 GxIAPI
        GxIAPI
    )
endif()

ament_target_dependencies(${PROJECT_NAME}_node
    rclcpp
    std_msgs
    auto_aim_common
    tracker_solver
    predictor
    detector
)

# 6. 安裝規則
if(TARGET ${PROJECT_NAME})
    install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_node
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION lib/${PROJECT_NAME}
    )
else()
    install(TARGETS ${PROJECT_NAME}_node
        RUNTIME DESTINATION lib/${PROJECT_NAME}
    )
endif()

# 如果有 include 資料夾則安裝
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/")
    install(DIRECTORY include/
        DESTINATION include
    )
    ament_export_include_directories(include)
endif()

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

# 7. 導出依賴
if(TARGET ${PROJECT_NAME})
    ament_export_libraries(${PROJECT_NAME})
endif()

ament_export_dependencies(
    rclcpp
    std_msgs
    auto_aim_common
    tracker_solver
    predictor
    detector
    OpenCV 
    Eigen3
    OpenVINO
)

ament_package()