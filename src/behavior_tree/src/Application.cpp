#include "../include/Application.hpp"
#include <filesystem>
#include <fstream>
#include <ament_index_cpp/get_package_share_directory.hpp>

#include "Node.hpp"
    
using namespace LangYa;
using namespace BehaviorTree;

namespace BehaviorTree {
    
    Application::Application(int argc, char **argv) {
        // 1. [ROS 2] 初始化 ROS Context
        // 雖然通常在 main 裡做，但在這裡確保 options 可以傳入
        if (!rclcpp::ok()) {
            rclcpp::init(argc, argv);
        }

        // 2. [ROS 2] 創建節點實例
        rclcpp::NodeOptions options;
        options.allow_undeclared_parameters(true);
        options.automatically_declare_parameters_from_overrides(true);
        node_ = std::make_shared<rclcpp::Node>(nodeName, options);

        // 3. 初始化日誌 (Logger)
        if(InitLogger()) {
            LoggerPtr->Info("Logger Init Success!");
            RCLCPP_INFO(node_->get_logger(), "Logger Init Success!");
        } else {
            // 如果日誌初始化失敗，至少用 ROS logger 報錯
            RCLCPP_ERROR(node_->get_logger(), "Logger Init Failed!");
            throw std::runtime_error("Logger Init Failed!");
        }

        // 4. [ROS 2] 動態獲取 Config 和 XML 路徑
        try {
            std::string pkg_path = ament_index_cpp::get_package_share_directory("behavior_tree");
            behavior_tree_file_ = pkg_path + "/Scripts/main.xml";
            config_file_       = pkg_path + "/Scripts/config.json";
            
            LoggerPtr->Info("Loading BT from: {}", behavior_tree_file_);
            LoggerPtr->Info("Loading Config from: {}", config_file_);
        } catch (const std::exception& e) {
            RCLCPP_ERROR(node_->get_logger(), "Error: Could not find package 'behavior_tree'.");
            throw e;
        }

        // 5. 初始化配置與通訊
        SubscribeMessageAll();  // 建立訂閱
        PublishMessageAll();    // 建立發布 (對應 GenPub)
        ConfigurationInit();    // 讀取 config.json

        // 6. 註冊與加載行為樹
        if(!RegisterTreeNodes()) {
            throw std::runtime_error("Behavior Tree Node Register Failed!");
        }
        
        // 建議這裡也把 LoadBehaviorTree 加上，除非你是在 GameLoop 裡動態加載
        if (!LoadBehaviorTree()) {
            LoggerPtr->Error("Behavior Tree Load Failed!");
             throw std::runtime_error("Behavior Tree Load Failed!");
        }

        LoggerPtr->Info("Application Start!");
    }

    Application::~Application() {
        if(LoggerPtr) {
            LoggerPtr->Info("Application Stop!");
            LoggerPtr->Flush();
        }
        // ROS 2 節點會由智能指針自動釋放
    }

    void Application::Run() {
        // 1. 等待比賽開始 (這通常會阻塞，直到收到裁判系統消息)
        WaitBeforeGame();

        // 2. 記錄比賽開始時間
        gameStartTime = std::chrono::steady_clock::now();

        // 3. 進入主循環 (處理業務邏輯和 BT Tick)
        GameLoop();
    }
}