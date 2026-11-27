import rclpy
from rclpy.node import Node
from rclpy.qos import qos_profile_sensor_data
from px4_msgs.msg import VehicleOdometry, VehicleStatus

class TelemetryNode(Node):
    def __init__(self):
        super().__init__('telemetry_node')
        self._odometry_sub = self.create_subscription(
            VehicleOdometry,
            '/fmu/out/vehicle_odometry',
            self.on_odometry,
            qos_profile_sensor_data
        )

        self._status_sub = self.create_subscription(
            VehicleStatus,
            '/fmu/out/vehicle_status_v4',
            self.on_status,
            qos_profile_sensor_data
        )

    def on_odometry(self, msg: VehicleOdometry):
        self.get_logger().info(f"X: {msg.position[0]:.2f}, Y: {msg.position[1]:.2f}, Z: {msg.position[2]:.2f}", throttle_duration_sec=1.0)

    def on_status(self, msg: VehicleStatus):
        self.get_logger().info( self.format_status(msg), throttle_duration_sec=1.0)

    def format_status(self, msg: VehicleStatus):
        if msg.nav_state == VehicleStatus.NAVIGATION_STATE_OFFBOARD and msg.arming_state == VehicleStatus.ARMING_STATE_ARMED:
            return "[FLYING]"
        if msg.nav_state == VehicleStatus.NAVIGATION_STATE_OFFBOARD:
            return "[OFFBOARD MODE ON]"
        if msg.arming_state == VehicleStatus.ARMING_STATE_ARMED:
            return "[ARMED]"
        else:
            return ""

def main(args=None):
    rclpy.init(args=args)
    telemetry_node = TelemetryNode()
    rclpy.spin(telemetry_node)

    telemetry_node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()