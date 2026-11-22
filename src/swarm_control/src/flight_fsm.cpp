#include "swarm_control/flight_fsm.hpp"

FSM::FSM() : current_state_(DroneState::MESSAGING), one_shot_event_(PonctualEvent::NONE), position_x_(0), position_y_(0), position_z_(0), xy_valid_(0), z_valid_(false), nav_state_offboard_(0), arm_state_armed_(0), command_acceptance_status_(0), landing_confirmed_(false), not_ready_for_offboard_mode_(true), counter_(0) {}

void FSM::set_xyz_xyz_valid(float a, float b, float c, bool d, bool e) {
        position_x_ = a;
        position_y_ = b;
        position_z_ = c;
        xy_valid_ = d;
        z_valid_ = e;
    }

void FSM::changer_etat() {
    counter_ ++;
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

        case DroneState::LAND :
            change_from_LAND();
            break;
    }
}

void FSM::change_from_MESSAGING() {
    if(!not_ready_for_offboard_mode_) {
        one_shot_event_ = PonctualEvent::SENDCOMMAND1;
        current_state_ = DroneState::COMMAND1;
        counter_ = 0;
        command_acceptance_status_ = false;
    } 
}

void FSM::change_from_COMMAND1() {
    one_shot_event_ = PonctualEvent::NONE;
    if(!command_acceptance_status_ && counter_ == 20) {
        one_shot_event_ = PonctualEvent::SENDCOMMAND1;
    }
    if(nav_state_offboard_) {
        one_shot_event_ = PonctualEvent::SENDCOMMAND2;
        current_state_ = DroneState::COMMAND2;
        counter_ = 0;
        command_acceptance_status_ = false;
    }
}

void FSM::change_from_COMMAND2() {
    one_shot_event_ = PonctualEvent::NONE;
    if(!command_acceptance_status_ && counter_ == 20) {
        one_shot_event_ = PonctualEvent::SENDCOMMAND2;
    }
    if(arm_state_armed_) {
        current_state_ = DroneState::CORNER1;
        counter_ = 0;
        command_acceptance_status_ = false;

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
        one_shot_event_ = PonctualEvent::SENDLAND;
        current_state_ = DroneState::LAND;
    }
}

void FSM::change_from_LAND() {
        one_shot_event_ = PonctualEvent::NONE;
}