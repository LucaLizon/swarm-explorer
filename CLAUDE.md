# swarm-explorer

Projet personnel : exploration en extérieur d'une zone définie par un essaim hétérogène de drones
(multirotors et voilures fixes), en simulation : couverture de zone, identification. Applications à préciser.
Plus tard : interface sol modulable (définition des objectifs, coordination de l'essaim), que je conçois moi-même ;
puis capacité à poursuivre la mission sans GPS (GPS-denied).
Objectif : portfolio pour des startups drones / robotique / spatial.

## Stack (versions figées)
- Ubuntu 24.04, ROS 2 Jazzy, Gazebo Harmonic, PX4 SITL (gz_x500), Micro XRCE-DDS Agent v2.4.3
- PX4-Autopilot sur `main` au commit e1c28e2755 (v1.18.0-beta1 + 995), pas sur une release : ne pas le mettre à jour sans refiger px4_msgs
- px4_msgs figé dans `dependencies.repos` (commit 7f7ebd2, généré depuis PX4 1a3f797, messages identiques à e1c28e2755)
- C++17 (nœuds principaux), Python (scripts, launch)

## Conventions
- PX4 utilise le repère NED (z vers le bas), ROS utilise ENU : conversions explicites et documentées
- Timestamps PX4 en microsecondes
- QoS best effort pour les topics /fmu/out (compatibles avec PX4)
- Certains topics PX4 sont versionnés (ex. /fmu/out/vehicle_status_v4) : vérifier avec `ros2 topic list`
- NAV_DLL_ACT=0 uniquement en simulation (pas de station sol)
- Conventional Commits, commits petits et logiques

## Organisation du dépôt
- La racine du dépôt est le workspace colcon ; le code est dans `src/`
- Dépendances externes : `vcs import src < dependencies.repos` (clonées dans `src/external/`, ignorées par git)
- Compilation : `colcon build` à la racine, puis `source install/setup.bash` dans chaque terminal
- Licence Apache-2.0

# État actuel
- Environnement validé : PX4 SITL + Gazebo, pont XRCE-DDS, exemple offboard de px4_ros_com fonctionnel
- Fait : px4_msgs figé et compilé dans le workspace ; package `swarm_control` créé (ament_cmake,
  dépend de rclcpp et px4_msgs) ; README ; doc d'architecture (`docs/architecture.md`)
- En cours (M1) : node `offboard_control`, machine à états décollage → carré de waypoints → atterrissage
  en mode offboard (x500)
  1. fait : subscription à `/fmu/out/vehicle_local_position_v1` (SensorDataQoS), position copiée dans des membres
  2. fait : 3 publishers `/fmu/in`, timer à 10 Hz, passage en offboard et armement ; le drone décolle à 5 m et y reste
  3. en cours : machine à états, classe `FlightFsm` sans dépendance ROS 2 (`flight_fsm.hpp/.cpp`, membre direct de la node) ;
     décidé : tolérance d'arrivée 0,5 m (à ajuster), retour à la position de départ avant l'atterrissage