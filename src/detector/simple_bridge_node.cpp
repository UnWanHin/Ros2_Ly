// 簡單橋接節點：將 predictor/outpost/buff 的目標直接轉發給 gimbal_driver
// 用途：測試用，看到就打（需在 yaml 中設置 test_mode.enable: true）
#include <rclcpp/rclcpp.hpp>
#include <auto_aim_common/msg/target.hpp>
#include <gimbal_driver/msg/gimbal_angles.hpp>
#include <std_msgs/msg/u_int8.hpp>

class SimpleBridgeNode : public rclcpp::Node {
public:
    SimpleBridgeNode() : Node("simple_bridge") {
        // 讀取測試模式參數
        declare_parameter("test_mode.enable", false);
        test_mode_enabled_ = get_parameter("test_mode.enable").as_bool();
        
        if (!test_mode_enabled_) {
            RCLCPP_INFO(get_logger(), "測試模式已關閉，等待 behavior_tree 決策");
            return;
        }
        
        // 訂閱三種模式的目標
        predictor_sub_ = create_subscription<auto_aim_common::msg::Target>(
            "/ly/predictor/target", 10,
            [this](const auto_aim_common::msg::Target::SharedPtr msg) { forward_target(msg); });
        
        outpost_sub_ = create_subscription<auto_aim_common::msg::Target>(
            "/ly/outpost/target", 10,
            [this](const auto_aim_common::msg::Target::SharedPtr msg) { forward_target(msg); });
        
        buff_sub_ = create_subscription<auto_aim_common::msg::Target>(
            "/ly/buff/target", 10,
            [this](const auto_aim_common::msg::Target::SharedPtr msg) { forward_target(msg); });
        
        // 發布控制指令
        angles_pub_ = create_publisher<gimbal_driver::msg::GimbalAngles>("/ly/control/angles", 10);
        fire_pub_ = create_publisher<std_msgs::msg::UInt8>("/ly/control/firecode", 10);
        
        RCLCPP_INFO(get_logger(), "測試模式已啟用 - 看到就打");
    }

private:
    void forward_target(const auto_aim_common::msg::Target::SharedPtr msg) {
        if (!test_mode_enabled_ || !msg->status) return;
        
        // 轉發角度
        gimbal_driver::msg::GimbalAngles angles;
        angles.yaw = msg->yaw;
        angles.pitch = msg->pitch;
        angles_pub_->publish(angles);
        
        // 發射！
        std_msgs::msg::UInt8 fire;
        fire.data = 1;  // 1 = 開火
        fire_pub_->publish(fire);
    }

    bool test_mode_enabled_ = false;
    rclcpp::Subscription<auto_aim_common::msg::Target>::SharedPtr predictor_sub_;
    rclcpp::Subscription<auto_aim_common::msg::Target>::SharedPtr outpost_sub_;
    rclcpp::Subscription<auto_aim_common::msg::Target>::SharedPtr buff_sub_;
    rclcpp::Publisher<gimbal_driver::msg::GimbalAngles>::SharedPtr angles_pub_;
    rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr fire_pub_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SimpleBridgeNode>());
    rclcpp::shutdown();
    return 0;
}
