#include "swarm_control/flight_fsm.hpp"

FSM::FSM() : position_x_(0), position_y_(0), position_z_(0), xy_valid_(0), z_valid_(false), current_state_(DroneState::MESSAGING) {}

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
    }
}


void FSM::change_from_MESSAGING() {
    ticker_for_vehicle_command_ ++;
    if(ticker_for_vehicle_command_ == 10) {
        current_state_ = DroneState::COMMAND1;
    }
}

void FSM::change_from_COMMAND1() {
    ticker_for_vehicle_command_ ++;
    if(ticker_for_vehicle_command_ == 12) {
        current_state_ = DroneState::COMMAND2;
    }
}

void FSM::change_from_COMMAND2() {
    current_state_ = DroneState::CORNER1;
}

void FSM::change_from_CORNER1() {
    if((position_x_ > 9 && position_x_ < 11) && (position_y_ > -1 && position_y_ < 1) && (position_z_ > -6 && position_z_ < -4) && xy_valid_ && z_valid_)
    current_state_ = DroneState::CORNER2;
}

void FSM::change_from_CORNER2() {
    if((position_x_ > 9 && position_x_ < 11) && (position_y_ > 9 && position_y_ < 11) && (position_z_ > -6 && position_z_ < -4) && xy_valid_ && z_valid_)
    current_state_ = DroneState::CORNER3;
}

void FSM::change_from_CORNER3() {
    if((position_x_ > -1 && position_x_ < 1) && (position_y_ > 9 && position_y_ < 11) && (position_z_ > -6 && position_z_ < -4) && xy_valid_ && z_valid_)
    current_state_ = DroneState::LAND;
}