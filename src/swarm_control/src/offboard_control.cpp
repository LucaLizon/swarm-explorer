#include "rclcpp/rclcpp.hpp"
#include <memory>
#include <chrono>
#include <limits> //Pour pouvoir ecrire std::numeric_limits<TypeName>::quiet_NaN() ¡¡Attention, TypeName a modifier dutant lecriture!!
#include "px4_msgs/msg/vehicle_local_position.hpp"
#include "px4_msgs/msg/offboard_control_mode.hpp"
#include "px4_msgs/msg/trajectory_setpoint.hpp"
#include "px4_msgs/msg/vehicle_command.hpp"
#include "px4_msgs/msg/vehicle_status.hpp"
#include "px4_msgs/msg/vehicle_command_ack.hpp"
#include "px4_msgs/msg/vehicle_land_detected.hpp"
#include "px4_msgs/msg/failsafe_flags.hpp"
#include "swarm_control/flight_fsm.hpp"

using namespace std::chrono_literals;

class OffboardControl : public rclcpp::Node {
public:
    OffboardControl() : Node("offboard_control") {
        //Subscriber Topic
        //vehicle_local_position_v1
        position_sub_ = this->create_subscription<px4_msgs::msg::VehicleLocalPosition>(
            "/fmu/out/vehicle_local_position_v1",
            rclcpp::SensorDataQoS(),
            [this](px4_msgs::msg::VehicleLocalPosition::SharedPtr msg){on_position(msg);}
        );
        //vehicle_status_v4
        status_sub_ = this->create_subscription<px4_msgs::msg::VehicleStatus>(
            "/fmu/out/vehicle_status_v4",
            rclcpp::SensorDataQoS(),
            [this](px4_msgs::msg::VehicleStatus::SharedPtr msg){on_status(msg);}
        );
        //vehicle_command_ack_v1
        command_ack_sub_ = this->create_subscription<px4_msgs::msg::VehicleCommandAck>(
            "/fmu/out/vehicle_command_ack_v1",
            rclcpp::SensorDataQoS(),
            [this](px4_msgs::msg::VehicleCommandAck::SharedPtr msg){on_ack(msg);}
        );
        //vehicle_land_detected
        land_detection_sub_ = this->create_subscription<px4_msgs::msg::VehicleLandDetected>(
            "/fmu/out/vehicle_land_detected",
            rclcpp::SensorDataQoS(),
            [this](px4_msgs::msg::VehicleLandDetected::SharedPtr msg){on_land_detection(msg);}
        );
        //failsafe_flags (to go from MESSAGING to COMMAND1)
        failsafe_flags_sub_ = this->create_subscription<px4_msgs::msg::FailsafeFlags>(
            "/fmu/out/failsafe_flags",
            rclcpp::SensorDataQoS(),
            [this](px4_msgs::msg::FailsafeFlags::SharedPtr msg){on_failsafe_flags(msg);}
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
    FSM fsm_{};

    //Subscribtion  
    std::shared_ptr<rclcpp::Subscription<px4_msgs::msg::VehicleLocalPosition>> position_sub_;
    std::shared_ptr<rclcpp::Subscription<px4_msgs::msg::VehicleStatus>> status_sub_;
    std::shared_ptr<rclcpp::Subscription<px4_msgs::msg::VehicleCommandAck>> command_ack_sub_;
    std::shared_ptr<rclcpp::Subscription<px4_msgs::msg::VehicleLandDetected>> land_detection_sub_;
    std::shared_ptr<rclcpp::Subscription<px4_msgs::msg::FailsafeFlags>> failsafe_flags_sub_;


    void on_position(px4_msgs::msg::VehicleLocalPosition::SharedPtr msg) {
        fsm_.set_xyz_xyz_valid(msg->x, msg->y,msg->z, msg->xy_valid,  msg->z_valid);
        RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 1000, "position_x_ %f	position_y_ %f	position_z_ %f	xy_valid_ %d z_valid_ %d", msg->x, msg->y, msg->z, msg->xy_valid, msg->z_valid);
    }

    void on_status(px4_msgs::msg::VehicleStatus::SharedPtr msg) {
        if(msg->nav_state == px4_msgs::msg::VehicleStatus::NAVIGATION_STATE_OFFBOARD) {
            fsm_.set_nav_state_offboard(true);
        } else {
            fsm_.set_nav_state_offboard(false);
        }
        if(msg->arming_state == px4_msgs::msg::VehicleStatus::ARMING_STATE_ARMED) {
            fsm_.set_arm_state_armed(true);
        } else {
            fsm_.set_arm_state_armed(false);
        }
    }

    void on_ack(px4_msgs::msg::VehicleCommandAck::SharedPtr msg) {
        if((msg->command == px4_msgs::msg::VehicleCommand::VEHICLE_CMD_DO_SET_MODE || 
            msg->command == px4_msgs::msg::VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM ||
            msg->command == px4_msgs::msg::VehicleCommand::VEHICLE_CMD_NAV_LAND) 
            && msg->result == 0 && msg->target_system == 1) {
            fsm_.set_command_acceptance_status(true);
        }
        else {
            fsm_.set_command_acceptance_status(false);
        }
    }

    void on_land_detection(px4_msgs::msg::VehicleLandDetected::SharedPtr msg) {
        fsm_.set_landing_confirmed(msg->landed);
    }

    void on_failsafe_flags(px4_msgs::msg::FailsafeFlags::SharedPtr msg) {
        fsm_.set_not_ready_for_offboard_mode_(msg->offboard_control_signal_lost);
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
        
        if(fsm_.get_current_state() == DroneState::MESSAGING) {
            publish_trajectory_set_point();
        }
        //OneShotEvent
        if(fsm_.get_one_shot_event_() == PonctualEvent::SENDCOMMAND1) {
                publish_vehicle_command(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_DO_SET_MODE, 1, 6);
        }

        if(fsm_.get_current_state() == DroneState::COMMAND1) {
            publish_trajectory_set_point();
        }
        //OneShotEvent
        if(fsm_.get_one_shot_event_() == PonctualEvent::SENDCOMMAND2) {
                publish_vehicle_command(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM, 1.0);
        }

        if(fsm_.get_current_state() == DroneState::COMMAND2) {
            publish_trajectory_set_point();
        }

        if(fsm_.get_current_state() == DroneState::CORNER1) {
            publish_trajectory_set_point(10, 0, -5);
        }

        if(fsm_.get_current_state() == DroneState::CORNER2) {
            publish_trajectory_set_point(10, 10, -5);
        }

        if(fsm_.get_current_state() == DroneState::CORNER3) {
            publish_trajectory_set_point(0, 10, -5);
        }

        if(fsm_.get_current_state() == DroneState::CORNER4) {
            publish_trajectory_set_point(0, 0, -5);
        }
        //OneShotEvent
        if(fsm_.get_one_shot_event_() == PonctualEvent::SENDLAND) {
                publish_vehicle_command(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_NAV_LAND);
        }

        if(fsm_.get_current_state() == DroneState::LANDED) {
            //Comportement a determiner
        }

        fsm_.changer_etat();
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

    void publish_vehicle_command(uint16_t command, float param1 = 0.0, float param2 = 0.0){
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
