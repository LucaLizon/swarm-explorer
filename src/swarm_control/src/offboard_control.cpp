#include "rclcpp/rclcpp.hpp"
#include <memory>
#include "px4_msgs/msg/vehicle_local_position.hpp"

class OffboardControl : public rclcpp::Node {
public:
    OffboardControl() : Node("offboard_control"),  position_x_(0), position_y_(0), position_z_(0), xy_valid_(false), z_valid_(false){
        position_sub_ = create_subscription<px4_msgs::msg::VehicleLocalPosition>(
            "/fmu/out/vehicle_local_position_v1",
            rclcpp::SensorDataQoS(),
            [this](px4_msgs::msg::VehicleLocalPosition::SharedPtr msg){on_position(msg);}
        );
    }

private: 
    std::shared_ptr<rclcpp::Subscription<px4_msgs::msg::VehicleLocalPosition>> position_sub_;

    float position_x_;
    float position_y_;
    float position_z_;

    bool xy_valid_;
    bool z_valid_;

    void on_position(px4_msgs::msg::VehicleLocalPosition::SharedPtr msg) {
        position_x_ = msg->x;
        position_y_ = msg->y;
        position_z_ = msg->z;

        xy_valid_ = msg->xy_valid;
        z_valid_ = msg->z_valid;

        RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 1000, "position_x_ %f	position_y_ %f	position_z_ %f	xy_valid_ %d z_valid_ %d", position_x_, position_y_, position_z_, xy_valid_, z_valid_);

    }
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<OffboardControl>());
    rclcpp::shutdown();
    return 0;
}
