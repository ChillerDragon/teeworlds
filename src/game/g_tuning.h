/* physics tuning */
MACRO_TUNING_PARAM(ground_control_speed, 350.0f / ticks_per_second)
MACRO_TUNING_PARAM(ground_control_accel, 100.0f / ticks_per_second)
MACRO_TUNING_PARAM(ground_friction, 0.5f)
MACRO_TUNING_PARAM(ground_jump_impulse, 12.6f)
MACRO_TUNING_PARAM(air_jump_impulse, 11.5f)
MACRO_TUNING_PARAM(air_control_speed, 250.0f / ticks_per_second)
MACRO_TUNING_PARAM(air_control_accel, 1.5f)
MACRO_TUNING_PARAM(air_friction, 0.95f)
MACRO_TUNING_PARAM(hook_length, 34*10.0f)
MACRO_TUNING_PARAM(hook_fire_speed, 45.0f)
MACRO_TUNING_PARAM(hook_drag_accel, 3.0f)
MACRO_TUNING_PARAM(hook_drag_speed, 15.0f)
MACRO_TUNING_PARAM(gravity, 0.5f)

MACRO_TUNING_PARAM(velramp_start, 500)
MACRO_TUNING_PARAM(velramp_range, 2000)
MACRO_TUNING_PARAM(velramp_curvature, 1.5f)

/* weapon tuning */
MACRO_TUNING_PARAM(gun_curvature, 1.5f)
MACRO_TUNING_PARAM(gun_speed, 2200.0f)

MACRO_TUNING_PARAM(shotgun_curvature, 1.5f)
MACRO_TUNING_PARAM(shotgun_speed, 2200.0f)

MACRO_TUNING_PARAM(grenade_curvature, 7.0f)
MACRO_TUNING_PARAM(grenade_speed, 900.0f)

MACRO_TUNING_PARAM(laser_reach, 800.0f)
MACRO_TUNING_PARAM(laser_bounce_delay, 150)
MACRO_TUNING_PARAM(laser_bounce_num, 1)
MACRO_TUNING_PARAM(laser_bounce_cost, 0)
MACRO_TUNING_PARAM(laser_damage, 6)
