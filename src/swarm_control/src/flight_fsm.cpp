#include "swarm_control/flight_fsm.hpp"

FSM::FSM() : position_x_(0), position_y_(0), position_z_(0), xy_valid_(0), z_valid_(false), nav_state_(0), arm_state_(0), command_acceptance_status_(0), landing_confirmed_(false), not_ready_for_offboard_mode_(true), current_state_(DroneState::MESSAGING) {}

void FSM::set_xyz_xyz_valid(float a, float b, float c, bool d, bool e) {
        position_x_ = a;
        position_y_ = b;
        position_z_ = c;
        xy_valid_ = d;
        z_valid_ = e;
    }

void FSM::changer_etat() {
    switch(current_state_) {
        
        case DroneState::MESSAGING :
            change_from_MESSAGING();
            break;
        
        case DroneState::COMMAND1 :
            change_from_COMMAND1();
            break;

        case DroneState::COMMAND2 :
            change_from_COMMAND2();       
            break;

        case DroneState::CORNER1 : 
            change_from_CORNER1();
            break;
        
        case DroneState::CORNER2 : 
            change_from_CORNER2();
            break;

        case DroneState::CORNER3 : 
            change_from_CORNER3();
            break;

        case DroneState::CORNER4 :
            change_from_CORNER4();
            break;
    }
}


void FSM::change_from_MESSAGING() {
    if(!not_ready_for_offboard_mode_) {
        current_state_ = DroneState::COMMAND1;
    }
}

void FSM::change_from_COMMAND1() {
    //ATTENTION : ici le 14 represente la valeur associee a px4_msgs::msg::VehicleStatus::NAVIGATION_STATE_OFFBOARD a changer plus tard
    if(nav_state_ == 14) {
        current_state_ = DroneState::COMMAND2;
    }
}

void FSM::change_from_COMMAND2() {
    if(arm_state_ == 2) {
        current_state_ = DroneState::CORNER1;
    }
}

void FSM::change_from_CORNER1() {
    if((position_x_ > 9 && position_x_ < 11) && (position_y_ > -1 && position_y_ < 1) && (position_z_ > -6 && position_z_ < -4) && xy_valid_ && z_valid_){
        current_state_ = DroneState::CORNER2;
    }
}

void FSM::change_from_CORNER2() {
    if((position_x_ > 9 && position_x_ < 11) && (position_y_ > 9 && position_y_ < 11) && (position_z_ > -6 && position_z_ < -4) && xy_valid_ && z_valid_){
        current_state_ = DroneState::CORNER3;
    }
}

void FSM::change_from_CORNER3() {
    if((position_x_ > -1 && position_x_ < 1) && (position_y_ > 9 && position_y_ < 11) && (position_z_ > -6 && position_z_ < -4) && xy_valid_ && z_valid_){
        current_state_ = DroneState::CORNER4;
    }
}

void FSM::change_from_CORNER4() {
    if((position_x_ > -1 && position_x_ < 1) && (position_y_ > -1 && position_y_ < 1) && (position_z_ > -6 && position_z_ < -4) && xy_valid_ && z_valid_){
        current_state_ = DroneState::LAND;
    }
}