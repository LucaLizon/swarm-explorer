#include "rclcpp/rclcpp.hpp"
#include <memory>
#include <chrono>
#include <limits> //Pour pouvoir ecrire std::numeric_limits<TypeName>::quiet_NaN() ¡¡Attention, TypeName a modifier dutant lecriture!!
#include "px4_msgs/msg/vehicle_local_position.hpp"
#include "px4_msgs/msg/offboard_control_mode.hpp"
#include "px4_msgs/msg/trajectory_setpoint.hpp"
#include "px4_msgs/msg/vehicle_command.hpp"
#include "swarm_control/flight_fsm.hpp"

using namespace std::chrono_literals;

class OffboardControl : public rclcpp::Node {
public:
    OffboardControl() : Node("offboard_control") {
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
    //Pointeur vers objet FSM
    FSM fsm{};

    //Subscribtion  
    std::shared_ptr<rclcpp::Subscription<px4_msgs::msg::VehicleLocalPosition>> position_sub_;

    void on_position(px4_msgs::msg::VehicleLocalPosition::SharedPtr msg) {
        fsm.set_xyz_xyz_valid(msg->x, msg->y,msg->z, msg->xy_valid,  msg->z_valid);
        RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 1000, "position_x_ %f	position_y_ %f	position_z_ %f	xy_valid_ %d z_valid_ %d", msg->x, msg->y, msg->z, msg->xy_valid, msg->z_valid);
    }

    //Publsisher
    std::shared_ptr<rclcpp::Publisher<px4_msgs::msg::OffboardControlMode>> offboard_control_mode_pub_;
    std::shared_ptr<rclcpp::Publisher<px4_msgs::msg::TrajectorySetpoint>> trajectory_setpoint_pub_;
    std::shared_ptr<rclcpp::Publisher<px4_msgs::msg::VehicleCommand>> vehicle_command_pub_;

    //Timer
    rclcpp::TimerBase::SharedPtr timer_;

    void timer_callback(){
        //OffboardControlMode
        publish_offboard_control_mode();

        //Gestion de la FSM
        if(fsm.get_current_state() == DroneState::MESSAGING) {
            publish_trajectory_set_point();
        }

        if(fsm.get_current_state() == DroneState::COMMAND1) {
            publish_vehicle_command(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_DO_SET_MODE, 1, 6);
            publish_trajectory_set_point();
        }

        if(fsm.get_current_state() == DroneState::COMMAND2) {
            publish_vehicle_command(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM, 1.0);
            publish_trajectory_set_point();
        }

        if(fsm.get_current_state() == DroneState::CORNER1) {
            publish_trajectory_set_point(10, 0, -5);
        }

        if(fsm.get_current_state() == DroneState::CORNER2) {
            publish_trajectory_set_point(10, 10, -5);
        }

        if(fsm.get_current_state() == DroneState::CORNER3) {
            publish_trajectory_set_point(0, 10, -5);
        }

        if(fsm.get_current_state() == DroneState::LAND) {
            publish_trajectory_set_point(0, 0, -5);
        }

        fsm.changer_etat();
    }

    void publish_offboard_control_mode() {
        //OffboardControlMode
        px4_msgs::msg::OffboardControlMode msg_offboard_ctrl{};

        msg_offboard_ctrl.position = true;
        msg_offboard_ctrl.velocity = false;
        msg_offboard_ctrl.acceleration = false;
        msg_offboard_ctrl.attitude = false;
        msg_offboard_ctrl.body_rate = false;
        msg_offboard_ctrl.thrust_and_torque = false;
        msg_offboard_ctrl.direct_actuator = false;
        msg_offboard_ctrl.timestamp = this->get_clock()->now().nanoseconds() / 1000;

        offboard_control_mode_pub_->publish(msg_offboard_ctrl);
    }

    void publish_trajectory_set_point(float x = 0, float y = 0, float z = -5, float yaw = 0) {
        //TrajectorySetpoint
        px4_msgs::msg::TrajectorySetpoint msg_trajectory_set_point{};

        msg_trajectory_set_point.position = {x, y, z};
	    msg_trajectory_set_point.yaw = yaw; // [-PI:PI]        
        msg_trajectory_set_point.timestamp = this->get_clock()->now().nanoseconds() / 1000;

        trajectory_setpoint_pub_->publish(msg_trajectory_set_point);
    }

    void publish_vehicle_command(uint16_t command, float param1, float param2 = 0.0){
        px4_msgs::msg::VehicleCommand msg_vehicle_command{};
        msg_vehicle_command.param1 = param1;
        msg_vehicle_command.param2 = param2;
        msg_vehicle_command.command = command;
        msg_vehicle_command.target_system = 1; 
        msg_vehicle_command.target_component = 1;
        msg_vehicle_command.source_system = 1;
        msg_vehicle_command.source_component = 1;
        msg_vehicle_command.from_external = true;
        msg_vehicle_command.timestamp = this->get_clock()->now().nanoseconds() / 1000;
        vehicle_command_pub_->publish(msg_vehicle_command);
    }
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<OffboardControl>());
    rclcpp::shutdown();
    return 0;
}
