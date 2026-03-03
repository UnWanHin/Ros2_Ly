// 簡單橋接節點：將 predictor/outpost/buff 的目標直接轉發給 gimbal_driver
// 用途：測試用，看到就打（需在 yaml 中設置 test_mode.enable: true）
#include <rclcpp/rclcpp.hpp>
#include <auto_aim_common/msg/target.hpp>
#include <gimbal_driver/msg/gimbal_angles.hpp>
#include <std_msgs/msg/u_int8.hpp>
#include <chrono>

class SimpleBridgeNode : public rclcpp::Node {
public:
    SimpleBridgeNode() : Node("simple_bridge") {
        // 讀取測試模式參數
        declare_parameter("test_mode.enable", false);
        declare_parameter("test_mode.force_fire_flip", false);
        declare_parameter("test_mode.fire_flip_hz", 8.0);
        test_mode_enabled_ = get_parameter("test_mode.enable").as_bool();
        force_fire_flip_ = get_parameter("test_mode.force_fire_flip").as_bool();
        fire_flip_hz_ = get_parameter("test_mode.fire_flip_hz").as_double();

        if (!test_mode_enabled_) {
            RCLCPP_INFO(get_logger(), "測試模式已關閉，等待 behavior_tree 決策");
            return;
        }

        if (fire_flip_hz_ <= 0.0) {
            RCLCPP_WARN(get_logger(), "test_mode.fire_flip_hz <= 0，改回 8.0 Hz");
            fire_flip_hz_ = 8.0;
        }

        // 發布控制指令
        angles_pub_ = create_publisher<gimbal_driver::msg::GimbalAngles>("/ly/control/angles", 10);
        fire_pub_ = create_publisher<std_msgs::msg::UInt8>("/ly/control/firecode", 10);

        if (force_fire_flip_) {
            // [臨時測試] 不依賴目標，直接持續翻轉火控位
            publish_flipped_firecode();
            const auto period = std::chrono::duration<double>(1.0 / fire_flip_hz_);
            force_fire_timer_ = create_wall_timer(
                std::chrono::duration_cast<std::chrono::nanoseconds>(period),
                [this]() { publish_flipped_firecode(); });
            RCLCPP_WARN(get_logger(),
                        "測試模式啟用(force_fire_flip=true): 持續翻轉 /ly/control/firecode, hz=%.2f",
                        fire_flip_hz_);
            return;
        }

        // 原有模式：看到目標才打
        predictor_sub_ = create_subscription<auto_aim_common::msg::Target>(
            "/ly/predictor/target", 10,
            [this](const auto_aim_common::msg::Target::SharedPtr msg) { forward_target(msg); });

        outpost_sub_ = create_subscription<auto_aim_common::msg::Target>(
            "/ly/outpost/target", 10,
            [this](const auto_aim_common::msg::Target::SharedPtr msg) { forward_target(msg); });

        buff_sub_ = create_subscription<auto_aim_common::msg::Target>(
            "/ly/buff/target", 10,
            [this](const auto_aim_common::msg::Target::SharedPtr msg) { forward_target(msg); });

        RCLCPP_INFO(get_logger(), "測試模式已啟用 - 看到就打");
    }

private:
    void publish_flipped_firecode() {
        fire_status_ = (fire_status_ == 0U) ? 0b11U : 0b00U;
        std_msgs::msg::UInt8 fire;
        fire.data = fire_status_;
        fire_pub_->publish(fire);
    }

    void forward_target(const auto_aim_common::msg::Target::SharedPtr msg) {
        if (!test_mode_enabled_ || force_fire_flip_ || !msg->status) return;

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
    bool force_fire_flip_ = false;
    double fire_flip_hz_ = 8.0;
    std::uint8_t fire_status_ = 0;

    rclcpp::Subscription<auto_aim_common::msg::Target>::SharedPtr predictor_sub_;
    rclcpp::Subscription<auto_aim_common::msg::Target>::SharedPtr outpost_sub_;
    rclcpp::Subscription<auto_aim_common::msg::Target>::SharedPtr buff_sub_;
    rclcpp::Publisher<gimbal_driver::msg::GimbalAngles>::SharedPtr angles_pub_;
    rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr fire_pub_;
    rclcpp::TimerBase::SharedPtr force_fire_timer_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SimpleBridgeNode>());
    rclcpp::shutdown();
    return 0;
}
