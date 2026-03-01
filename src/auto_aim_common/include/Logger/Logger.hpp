// auto_aim_common/include/Logger/Logger.hpp
#pragma once

#include <rclcpp/rclcpp.hpp>
#include <string>
#include <cstdio>  // 使用 C-style 格式化避免 fmt 衝突

namespace roslog {

inline rclcpp::Logger get_logger() {
    static const rclcpp::Logger logger = rclcpp::get_logger("LY");
    return logger;
}

// 使用 C++11 可變參數模板 + snprintf 避開 fmt 問題
template<typename... Args>
inline void error(const char* fmt, Args&&... args) {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), fmt, std::forward<Args>(args)...);
    RCLCPP_ERROR(get_logger(), "%s", buffer);
}

template<typename... Args>
inline void warn(const char* fmt, Args&&... args) {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), fmt, std::forward<Args>(args)...);
    RCLCPP_WARN(get_logger(), "%s", buffer);
}

template<typename... Args>
inline void info(const char* fmt, Args&&... args) {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), fmt, std::forward<Args>(args)...);
    RCLCPP_INFO(get_logger(), "%s", buffer);
}

template<typename... Args>
inline void debug(const char* fmt, Args&&... args) {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), fmt, std::forward<Args>(args)...);
    RCLCPP_DEBUG(get_logger(), "%s", buffer);
}

// 重載無參數版本
inline void error(const char* msg) { RCLCPP_ERROR(get_logger(), "%s", msg); }
inline void warn(const char* msg)  { RCLCPP_WARN(get_logger(), "%s", msg); }
inline void info(const char* msg)  { RCLCPP_INFO(get_logger(), "%s", msg); }
inline void debug(const char* msg) { RCLCPP_DEBUG(get_logger(), "%s", msg); }

// 重載 std::string 版本
inline void error(const std::string& msg) { RCLCPP_ERROR(get_logger(), "%s", msg.c_str()); }
inline void warn(const std::string& msg)  { RCLCPP_WARN(get_logger(), "%s", msg.c_str()); }
inline void info(const std::string& msg)  { RCLCPP_INFO(get_logger(), "%s", msg.c_str()); }
inline void debug(const std::string& msg) { RCLCPP_DEBUG(get_logger(), "%s", msg.c_str()); }

} // namespace roslog