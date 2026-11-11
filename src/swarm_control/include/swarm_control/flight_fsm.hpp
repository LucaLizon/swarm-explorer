#pragma once // VITAL
#include <stdint.h>

enum class DroneState {MESSAGING, COMMAND1, COMMAND2, CORNER1, CORNER2, CORNER3, LAND};

class FSM {
private:
    DroneState current_state_;

    float position_x_;
    float position_y_;
    float position_z_;

    bool xy_valid_;
    bool z_valid_;

    uint8_t nav_state_;
    uint8_t arm_state_;

    uint8_t command_acceptance_status_;

    bool landing_confirmed_;

    int ticker_for_vehicle_command_ = 0;

public:

    FSM();

    void set_xyz_xyz_valid(float a, float b, float c, bool d, bool e);

    void changer_etat();

    DroneState get_current_state() const {return current_state_;}

    void set_nav_state(uint8_t a) {nav_state_ = a;}
    void set_arm_state(uint8_t b) {arm_state_ = b;}
    void set_command_acceptance_status(uint8_t c) {command_acceptance_status_ = c;}
    void set_landing_confirmed(bool d) {landing_confirmed_ = d;}


    void change_from_MESSAGING();
    void change_from_COMMAND1();
    void change_from_COMMAND2();
    void change_from_CORNER1();
    void change_from_CORNER2();
    void change_from_CORNER3();

};