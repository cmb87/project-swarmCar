import math


# https://en.wikipedia.org/wiki/Rolling_resistance

def drive_train_design(wheel_diameter, desired_speed, total_weight, voltage, inclination_angle=0, gearRatio=1):
    # Constants
    efficiency = 0.9  # assumed motor efficiency
    gravity = 9.81  # acceleration due to gravity in m/s^2

    # Convert speed to angular velocity (rad/s)
    wheel_radius = wheel_diameter / 2
    angular_velocity = desired_speed / wheel_radius
    rpm = angular_velocity*9.5493 *gearRatio

    # Calculate rolling resistance torque (Nm) with inclination
    rolling_resistance = 0.01  # coefficient of rolling resistance (adjust as needed)
    rolling_resistance_torque = rolling_resistance * total_weight * wheel_radius * math.cos(math.atan(inclination_angle))

    # Calculate gravitational force torque (Nm)
    gravitational_torque = total_weight * wheel_radius * gravity * math.sin(math.atan(inclination_angle))

    # Calculate total torque (Nm)
    torque = rolling_resistance_torque + gravitational_torque

    # Calculate power (Watts)
    power = torque * angular_velocity / efficiency

    # Calculate current (Amperes)
    current = power / voltage

    return power, torque, current, rpm

# Example usage
wheel_diameter = 0.25  # Replace with your actual wheel diameter in meters
desired_speed = 1.4  # Replace with your desired speed in m/s
total_weight = 20.0  # Replace with the total weight of the mobile platform in kg
voltage = 12.0  # Operating voltage in volts
inclination_angle = math.radians(10)  # Replace with the inclination angle in radians
gearRatio = 10

print(inclination_angle)
power, torque, current, rpm = drive_train_design(wheel_diameter, desired_speed, total_weight, voltage, inclination_angle, gearRatio)

# Display the results
print(f"Required Motor Power:   {power:.2f} W")
print(f"Required Torque:        {torque:.2f} Nm")
print(f"Required Current:       {current:.2f} Amperes")
print(f"RPM:                    {rpm:.2f} rpm")