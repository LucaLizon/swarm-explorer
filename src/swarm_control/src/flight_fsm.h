#pragma once

enum class DroneState { MESSAGING, OFFBOARD };

class FSM {
private:
    float position_x_;
    float position_y_;
    float position_z_;

    bool xy_valid_;
    bool z_valid_;

    int ticker_for_vehicle_command_ = 0;

public:
    FSM() : position_x_(0), position_y_(0), position_z_(0), xy_valid_(0), z_valid_(false) {}

    void set_xyz_xyz_valid(float a, float b, float c, bool d, bool e) {
        position_x_ = a;
        position_y_ = b;
        position_z_ = c;
        xy_valid_ = d;
        z_valid_ = e;
    }

    void changer_etat();
};
