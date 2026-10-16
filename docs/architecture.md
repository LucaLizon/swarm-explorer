# Architecture : de Gazebo à la node ROS 2

Ce document décrit comment les composants de la simulation communiquent, le rôle de chacun,
et les commandes qui permettent d'observer ou d'agir sur chaque partie.

## 1. Vue d'ensemble

```
┌──────────────┐                 ┌──────────────────────────────────────┐
│   GAZEBO     │   gz-transport  │                PX4                   │
│ (monde       │──capteurs──────▶│  gz_bridge ─▶ ekf2 ─▶ commander      │
│  simulé)     │◀──moteurs───────│        régulateurs ◀─┘               │
│              │                 │  (modules reliés par uORB)           │
│ [gz topic]   │                 │  uxrce_dds_client                    │
└──────────────┘                 │  [console pxh> : listener, commander]│
                                 └──────────────────┬───────────────────┘
                                                    │ UDP, port 8888
                                                    │ (protocole XRCE)
                                 ┌──────────────────▼───────────────────┐
                                 │        AGENT XRCE-DDS                │
                                 │  [MicroXRCEAgent udp4 -p 8888]       │
                                 └──────────────────┬───────────────────┘
                                                    │ DDS
                        /fmu/out/...  ◀─────────────┼─────────────▶  /fmu/in/...
                                                    │
                        ┌───────────────────────────┴──────────────────┐
                        │              ROS GRAPH                       │
                        │  node offboard_control     [ros2 run]        │
                        │  outils ros2 [ros2 topic / node / interface] │
                        └──────────────────────────────────────────────┘
```

Entre crochets : les commandes qui agissent sur chaque partie (détaillées en section 6).

## 2. Trois middlewares, chacun avec ses topics

Un **middleware** est la couche logicielle qui transporte les messages : découverte des participants,
sérialisation, envoi, files d'attente. La simulation en utilise trois, et chacun a ses propres topics.
Un topic d'un middleware n'existe pas dans un autre tant qu'une passerelle ne le recopie pas.

| Middleware | Où | Exemples de topics | Commande pour les lister |
|---|---|---|---|
| **gz-transport** | entre Gazebo et PX4 | mesures capteurs, commandes moteurs | `gz topic -l` |
| **uORB** | à l'intérieur de PX4 | `vehicle_local_position`, `vehicle_status` | `uorb top` (console `pxh>`) |
| **DDS** | le monde ROS 2 | `/fmu/out/vehicle_local_position_v1` | `ros2 topic list` |

Exemple : le **uORB topic** `vehicle_local_position` (dans PX4) et le **ROS 2 topic**
`/fmu/out/vehicle_local_position_v1` (dans DDS) sont deux topics distincts. La passerelle
(client XRCE + agent) recopie le premier dans le second.

## 3. Rôle de chaque composant

### Gazebo

Le banc d'essai : il simule le drone et son environnement. À chaque pas de calcul, il produit les
mesures des capteurs (centrale inertielle, baromètre, magnétomètre) et applique la poussée des moteurs.
Il échange avec PX4 par gz-transport. Sur un vrai drone, cette partie disparaît : capteurs et moteurs
sont réels.

### PX4

Le pilote automatique. Il est découpé en modules qui communiquent par **uORB**, un middleware
publish/subscribe interne au processus, en mémoire partagée. Les modules concernés :

- **`gz_bridge`** : se branche sur gz-transport, reçoit les capteurs simulés et renvoie les commandes moteurs.
- **`ekf2`** (l'estimateur) : fusionne les mesures et publie la position estimée sur le uORB topic
  `vehicle_local_position`.
- **`commander`** : gère les modes de vol et l'armement, et publie `vehicle_status`.
- **les régulateurs** : lisent le setpoint et la position, et calculent les commandes moteurs.
- **`uxrce_dds_client`** : la moitié PX4 de la passerelle vers ROS 2.

La console `pxh>` agit directement sur uORB : `commander takeoff` fonctionne sans ROS 2 ni agent.

### La passerelle : client XRCE + agent

PX4 est conçu pour un microcontrôleur, trop limité pour faire tourner DDS. Il utilise **XRCE-DDS**,
une version allégée (XRCE = *eXtremely Resource Constrained Environments*), en deux moitiés :

- **Le client, dans PX4** (`uxrce_dds_client`) : il prend une liste fixe de uORB topics (définie dans
  `src/modules/uxrce_dds_client/dds_topics.yaml` de PX4), les sérialise et les envoie à l'agent par UDP
  sur le port 8888. Il reçoit aussi dans l'autre sens.
- **L'agent, sur le PC** (`MicroXRCEAgent`) : un participant DDS complet, qui crée les publishers et
  subscribers DDS pour le compte de PX4.

Le préfixe des noms donne le sens, vu depuis PX4 :

- `/fmu/out/...` : PX4 → ROS 2 (FMU = *Flight Management Unit*, la carte de vol) ;
- `/fmu/in/...` : ROS 2 → PX4.

### Le ROS graph et la node `offboard_control`

Le **ROS graph** est l'ensemble des nodes et des topics présents à un instant donné, avec qui publie
et qui s'abonne à quoi. Le middleware est DDS.

La node `offboard_control` (package `swarm_control`) vit là. Pour elle, PX4 n'est qu'un ensemble de
topics : elle ne connaît ni uORB, ni Gazebo, ni l'agent.

Les outils `ros2 topic ...` sont eux-mêmes de petites nodes temporaires, qui rejoignent le ROS graph
le temps de leur exécution.

## 4. Parcours d'un message

**Une position, de Gazebo à la node :**

1. Gazebo calcule le mouvement et publie les mesures capteurs sur gz-transport.
2. `gz_bridge` les reçoit dans PX4 ; `ekf2` publie la position sur le uORB topic
   `vehicle_local_position`, environ 50 fois par seconde.
3. `uxrce_dds_client` l'envoie par UDP à l'agent.
4. L'agent la publie dans DDS sur `/fmu/out/vehicle_local_position_v1`.
5. La subscription de la node la reçoit, et l'executor appelle son callback.

**Un setpoint, de la node aux moteurs**, fait le chemin inverse : publisher de la node →
`/fmu/in/trajectory_setpoint` → agent → `uxrce_dds_client` → uORB → régulateurs → `gz_bridge` →
moteurs simulés.

## 5. Topics utilisés par `offboard_control` (M1)

| Topic | Sens | Côté node | Rôle |
|---|---|---|---|
| `/fmu/out/vehicle_local_position_v1` | PX4 → node | subscription | position du drone (sortie d'`ekf2`) |
| `/fmu/out/vehicle_status_v4` | PX4 → node | subscription | mode de vol actif, armé ou non |
| `/fmu/in/offboard_control_mode` | node → PX4 | publisher | signal de vie : « je commande, en position » |
| `/fmu/in/trajectory_setpoint` | node → PX4 | publisher | le point à atteindre |
| `/fmu/in/vehicle_command` | node → PX4 | publisher | ordres ponctuels : offboard, armement, atterrissage |

Les suffixes `_v1` et `_v4` sont les versions des messages (`MESSAGE_VERSION` dans les `.msg`) ;
ils doivent correspondre à la version de px4_msgs figée dans `dependencies.repos`.

## 6. Commandes

### Lancer chaque partie (un terminal externe par ligne)

| Commande | Ce qu'elle démarre |
|---|---|
| `cd ~/PX4-Autopilot && make px4_sitl gz_x500` | Gazebo + PX4 ; le terminal devient la console `pxh>` |
| `MicroXRCEAgent udp4 -p 8888` | l'agent, en écoute UDP sur le port 8888 |
| `cd ~/swarm-explorer && source install/setup.bash` | rend le workspace visible dans ce terminal (une fois par terminal) |
| `ros2 run swarm_control offboard_control` | la node : `ros2 run <package> <exécutable>` |

L'ordre de lancement importe peu : le client XRCE réessaie tant que l'agent n'est pas là, et DDS
découvre les participants au fil de l'eau.

Ne pas lancer Gazebo depuis le terminal intégré de VS Code installé en snap : les variables
d'environnement du snap (`GTK_PATH`, `LOCPATH`…) font échouer le chargement de l'interface graphique
(`symbol lookup error ... libpthread.so.0`).

### Côté PX4 (console `pxh>`, donc côté uORB)

| Commande | Effet |
|---|---|
| `uorb top` | liste les uORB topics actifs et leur fréquence (`q` pour quitter) |
| `listener vehicle_local_position` | affiche un message d'un uORB topic |
| `uxrce_dds_client status` | état du client XRCE : connecté ou non à l'agent |
| `commander check` | vérifications avant armement, et ce qui bloque |
| `commander takeoff` / `commander land` | décollage / atterrissage, sans ROS 2 |
| `param show MIS_TAKEOFF_ALT` / `param set MIS_TAKEOFF_ALT 5` | lire / modifier un paramètre |

### Côté ROS 2 (terminal où le workspace est sourcé, donc côté DDS)

| Commande | Effet |
|---|---|
| `ros2 node list` | les nodes présentes dans le ROS graph |
| `ros2 topic list` | les topics présents |
| `ros2 topic info -v <topic>` | publishers et subscribers d'un topic, avec leur QoS |
| `ros2 topic echo <topic> --once` | affiche un message puis s'arrête |
| `ros2 topic echo <topic> --field z` | affiche seulement un champ, en continu (`Ctrl+C`) |
| `ros2 topic hz <topic>` | fréquence réelle de réception |
| `ros2 interface show px4_msgs/msg/VehicleLocalPosition` | définition du message (le `.msg`) |

### Côté Gazebo

| Commande | Effet |
|---|---|
| `gz topic -l` | les topics gz-transport |

### Diagnostic : remonter la chaîne

Si la node ne reçoit rien, remonter le schéma section par section :

1. `uorb top` dans `pxh>` : PX4 produit-il la position ?
2. `uxrce_dds_client status` dans `pxh>` : le client est-il connecté à l'agent ?
3. `ros2 topic hz /fmu/out/vehicle_local_position_v1` : la position arrive-t-elle dans DDS ?
4. `ros2 topic info -v` sur ce topic : la node apparaît-elle parmi les subscribers, avec une QoS compatible ?

La première étape qui échoue indique où chercher.

## 7. QoS du topic de position

Sortie de `ros2 topic info -v /fmu/out/vehicle_local_position_v1` (extrait) :

```
Node name: _CREATED_BY_BARE_DDS_APP_
Topic type hash: INVALID
QoS profile:
  Reliability: BEST_EFFORT
  History (Depth): UNKNOWN
  Durability: TRANSIENT_LOCAL
```

- **`_CREATED_BY_BARE_DDS_APP_`** : le publisher n'est pas une node ROS 2 ; l'agent l'a créé directement
  dans DDS. Sans incidence pour les subscribers : un topic ne dépend que de son nom et de son type.
- **`Topic type hash: INVALID`** : depuis Jazzy, ROS 2 calcule une empreinte de chaque type de message
  pour détecter les incompatibilités. L'agent ne la fournit pas, donc aucune vérification automatique
  n'a lieu. La cohérence repose sur px4_msgs figé au bon commit.
- **`Reliability: BEST_EFFORT`** : envoi sans acquittement ni retransmission. Un message perdu est
  remplacé 20 ms plus tard par le suivant.
- **`Durability: TRANSIENT_LOCAL`** : le publisher garde le dernier message et le remet à un subscriber
  qui arrive après coup. Une node reçoit donc une position dès son démarrage.
- **`History (Depth): UNKNOWN`** : la profondeur de file du publisher n'est pas annoncée.

**Règle de compatibilité** : un subscriber ne peut pas exiger plus que ce que le publisher offre.

- **Reliability** : PX4 offre `BEST_EFFORT`, la subscription doit être `BEST_EFFORT`. Le réglage par
  défaut de ROS 2 est `RELIABLE` : avec lui, la connexion ne s'établit pas et rien n'est reçu
  (seul un avertissement « incompatible QoS » le signale).
- **Durability** : PX4 offre `TRANSIENT_LOCAL`, le niveau le plus élevé. La subscription peut demander
  `VOLATILE` ou `TRANSIENT_LOCAL`.

Le préréglage `rclcpp::SensorDataQoS()` (`BEST_EFFORT`, `VOLATILE`, profondeur 5) convient.
`ros2 topic echo` s'adapte seul à la QoS du publisher, et `ros2 topic hz` utilise ce même préréglage.

## 8. Transposition sur un vrai drone

PX4 tourne sur la carte de vol ; l'agent et la node tournent sur un ordinateur embarqué
(*companion computer*), relié par câble série ou Ethernet. Gazebo disparaît. La node reste identique,
puisqu'elle ne voit que des topics : le travail fait en simulation est directement transposable.
