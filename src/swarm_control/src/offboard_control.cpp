#include "rclcpp/rclcpp.hpp"
#include <memory>
#include "px4_msgs/msg/vehicle_local_position.hpp"
#include <chrono>
#include "px4_msgs/msg/offboard_control_mode.hpp"
#include "px4_msgs/msg/trajectory_setpoint.hpp"
#include "px4_msgs/msg/vehicle_command.hpp"

using namespace std::chrono_literals;

class OffboardControl : public rclcpp::Node {
public:
    OffboardControl() : Node("offboard_control"),  position_x_(0), position_y_(0), position_z_(0), xy_valid_(false), z_valid_(false){
        //Subscriber Topic
        position_sub_ = this->create_subscription<px4_msgs::msg::VehicleLocalPosition>(
            "/fmu/out/vehicle_local_position_v1",
            rclcpp::SensorDataQoS(),
            [this](px4_msgs::msg::VehicleLocalPosition::SharedPtr msg){on_position(msg);}
        );


        //Publisher Topics
        offboard_control_mode_pub_ = this->create_publisher<px4_msgs::msg::OffboardControlMode>(
            "/fmu/in/offboard_control_mode", 10
        );
        trajectory_setpoint_pub_ = this->create_publisher<px4_msgs::msg::TrajectorySetpoint>(
            "/fmu/in/trajectory_setpoint", 10
        );
        vehicle_command_pub_ = this->create_publisher<px4_msgs::msg::VehicleCommand>(
            "/fmu/in/vehicle_command", 10
        );

        //Timer
        timer_ = this->create_wall_timer( 
            100ms, 
            [this](){this->timer_callback();}
        );
    }

private:
    //Subscribtion  
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

    //Publsisher
    std::shared_ptr<rclcpp::Publisher<px4_msgs::msg::OffboardControlMode>> offboard_control_mode_pub_;
    std::shared_ptr<rclcpp::Publisher<px4_msgs::msg::TrajectorySetpoint>> trajectory_setpoint_pub_;
    std::shared_ptr<rclcpp::Publisher<px4_msgs::msg::VehicleCommand>> vehicle_command_pub_;

    //Timer
    rclcpp::TimerBase::SharedPtr timer_;

    void timer_callback(){

        //OffboardControlMode
        px4_msgs::msg::OffboardControlMode msg_offboard_ctrl_{};

        msg_offboard_ctrl_.position = true;
        msg_offboard_ctrl_.velocity = false;
        msg_offboard_ctrl_.acceleration = false;
        msg_offboard_ctrl_.attitude = false;
        msg_offboard_ctrl_.body_rate = false;
        msg_offboard_ctrl_.thrust_and_torque = false;
        msg_offboard_ctrl_.direct_actuator = false;
        msg_offboard_ctrl_.timestamp = this->get_clock()->now().nanoseconds() / 1000;

        offboard_control_mode_pub_->publish(msg_offboard_ctrl_);

        //TrajectorySetpoint
        px4_msgs::msg::TrajectorySetpoint msg_trajectory_set_point_{};

        msg_trajectory_set_point_.position = {0.0, 0.0, -5.0};
	    msg_trajectory_set_point_.yaw = -3.14; // [-PI:PI]        
        msg_offboard_ctrl_.timestamp = this->get_clock()->now().nanoseconds() / 1000;

        trajectory_setpoint_pub_->publish(msg_trajectory_set_point_);

        //Vehicle Command
        

    }
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<OffboardControl>());
    rclcpp::shutdown();
    return 0;
}