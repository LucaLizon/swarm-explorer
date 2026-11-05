#pragma once // VITAL

enum class DroneState {MESSAGING, COMMAND1, COMMAND2, CORNER1, CORNER2, CORNER3, LAND};

class FSM {
private:
    DroneState current_state_;

    float position_x_;
    float position_y_;
    float position_z_;

    bool xy_valid_;
    bool z_valid_;

    int ticker_for_vehicle_command_ = 0;

public:

    FSM();

    void set_xyz_xyz_valid(float a, float b, float c, bool d, bool e);

    void changer_etat();

    DroneState get_current_state() const {return current_state_;}

    void change_from_MESSAGING();
    void change_from_COMMAND1();
    void change_from_COMMAND2();
    void change_from_CORNER1();
    void change_from_CORNER2();
    void change_from_CORNER3();

};