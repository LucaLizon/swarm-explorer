#pragma once // VITAL
#include <stdint.h>

enum class DroneState {MESSAGING, COMMAND1, COMMAND2, CORNER1, CORNER2, CORNER3, CORNER4, LAND, LANDED};
enum class PonctualEvent{NONE, SENDCOMMAND1, SENDCOMMAND2, SENDLAND};

class FSM {
private:
    DroneState current_state_;

    PonctualEvent one_shot_event_;

    float position_x_;
    float position_y_;
    float position_z_;

    bool xy_valid_;
    bool z_valid_;

    bool nav_state_offboard_;
    bool arm_state_armed_;

    bool command_acceptance_status_;

    bool landing_confirmed_;

    bool not_ready_for_offboard_mode_;

    int counter_;

public:

    FSM();

    void set_xyz_xyz_valid(float a, float b, float c, bool d, bool e);

    void changer_etat();

    DroneState get_current_state() const {return current_state_;}

    PonctualEvent get_one_shot_event_() {return one_shot_event_;}

    void set_nav_state_offboard(bool a) {nav_state_offboard_ = a;}
    void set_arm_state_armed(bool b) {arm_state_armed_ = b;}
    void set_command_acceptance_status(bool c) {command_acceptance_status_ = c;}
    void set_landing_confirmed(bool d) {landing_confirmed_ = d;}
    void set_not_ready_for_offboard_mode_(bool e) {not_ready_for_offboard_mode_ = e;}


    void change_from_MESSAGING();
    void change_from_COMMAND1();
    void change_from_COMMAND2();
    void change_from_CORNER1();
    void change_from_CORNER2();
    void change_from_CORNER3();
    void change_from_CORNER4();
    void change_from_LAND();
};