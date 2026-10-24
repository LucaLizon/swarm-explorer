# swarm-explorer

Outdoor exploration of a defined area by a heterogeneous swarm of drones (multirotors and
fixed-wing aircraft), in simulation: area coverage, identification of points of interest.

The project builds the full stack step by step: offboard control of a single PX4 drone from ROS 2,
then multi-vehicle coordination, and later a modular ground control interface to define mission
objectives and supervise the swarm. A later phase will address mission continuity in GPS-denied
conditions.

## Status

Early development. Current milestone: a ROS 2 node that reads the drone position and flies
a takeoff, square trajectory and landing sequence in offboard mode.

## Stack

- Ubuntu 24.04, ROS 2 Jazzy
- PX4 SITL with Gazebo Harmonic (x500 quadcopter)
- Micro XRCE-DDS Agent (PX4 to ROS 2 bridge)
- C++17 for the main nodes, Python for scripts and launch files

## Repository layout

```
dependencies.repos   external dependencies, pinned to exact commits (px4_msgs)
docs/                design notes (architecture.md, in French)
src/swarm_control/   ROS 2 package: offboard control node
```

## Build

Requires ROS 2 Jazzy and a PX4-Autopilot checkout matching the pinned `px4_msgs` version.

```bash
git clone https://github.com/LucaLizon/swarm-explorer.git
cd swarm-explorer
vcs import src < dependencies.repos
colcon build
source install/setup.bash
```

## License

Apache-2.0
