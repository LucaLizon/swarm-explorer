# swarm-explorer

Projet personnel : exploration autonome d'environnements intérieurs sans GPS par un essaim de drones,
en simulation. Objectif : portfolio pour des startups drones / robotique / spatial.

## Stack (versions figées)
- Ubuntu 24.04, ROS 2 Jazzy, Gazebo Harmonic, PX4 SITL (gz_x500), Micro XRCE-DDS Agent v2.4.3
- px4_msgs doit correspondre à la version de PX4 utilisée (à figer via un fichier .repos)
- C++17 (nœuds principaux), Python (scripts, launch)

## Conventions
- PX4 utilise le repère NED (z vers le bas), ROS utilise ENU : conversions explicites et documentées
- Timestamps PX4 en microsecondes
- QoS best effort pour les topics /fmu/out (compatibles avec PX4)
- Certains topics PX4 sont versionnés (ex. /fmu/out/vehicle_status_v4) : vérifier avec `ros2 topic list`
- NAV_DLL_ACT=0 uniquement en simulation (pas de station sol)
- Conventional Commits, commits petits et logiques

# État actuel
- Environnement validé : PX4 SITL + Gazebo, pont XRCE-DDS, exemple offboard de px4_ros_com fonctionnel
- Prochaine étape (M1) : écrire mon propre nœud dans swarm_control (lecture de la position,
  machine à états décollage → carré de waypoints → atterrissage)