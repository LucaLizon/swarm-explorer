# offboard_control node: topics

Topics published and subscribed by the `offboard_control` node (M1).

```mermaid
flowchart LR
    n1["offboard_control"] --> n2["/fmu/in/offboard_control_mode"] & n4["/fmu/in/trajectory_setpoint"] & n5["/fmu/in/vehicle_command"]
    n2 --> n6["MicroXRCEAgent"]
    n4 --> n6
    n5 --> n6
    n6 <--> n8["PX4"]
    n6 --> n7["/fmu/out/vehicle_local_position_v1"] & n9@{ label: "<span style=\"color:\">/fmu/out/vehicle_status_v4</span>" } & n10@{ label: "<span style=\"color:\">/fmu/out/vehicle_command_ack</span>" } & n11@{ label: "<span style=\"color:\">/fmu/out/vehicle_land_detected</span>" } & n12@{ label: "<span style=\"color:\">/fmu/out/vehicle_land_detected</span>" }
    n7 --> n1
    n9 --> n1
    n10 --> n1
    n11 --> n1
    n9 --> n1

    n1@{ shape: proc}
    n2@{ shape: in-out}
    n4@{ shape: in-out}
    n5@{ shape: in-out}
    n6@{ shape: proc}
    n8@{ shape: proc}
    n7@{ shape: out-in}
    n9@{ shape: out-in}
    n10@{ shape: out-in}
    n11@{ shape: out-in}
    n12@{ shape: out-in}
    style n1 fill:#BBDEFB,stroke-width:0px
    style n2 fill:#FFD600,stroke-width:0px
    style n4 fill:#FFD600,stroke-width:0px
    style n5 fill:#FFD600,stroke-width:0px
    style n6 fill:#00C853,stroke-width:0px
    style n8 fill:#FF6D00,stroke-width:0px
    style n7 fill:#FFE0B2,stroke-width:0px
    style n9 fill:#FFE0B2,stroke-width:0px
    style n10 fill:#FFE0B2,stroke-width:0px
    style n11 fill:#FFE0B2,stroke-width:0px
    style n12 fill:#FFE0B2,stroke-width:0px
```
