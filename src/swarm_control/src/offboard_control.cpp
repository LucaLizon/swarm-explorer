#include "rclcpp/rclcpp.hpp"
#include <memory>
#include "px4_msgs/msg/vehicle_local_position.hpp"

class OffboardControl : public rclcpp::Node {
public:
    OffboardControl() : Node("OffboardControl") {

    }

private: 
    std::shared_ptr<rclcpp::Subscription<px4_msgs::msg::VehicleLocalPosition>> position_sub;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<OffboardControl>());
    rclcpp::shutdown();
    return 0;
}
