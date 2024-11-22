
#ifndef FOC_H
#define FOC_H

#include "arm_math.h"
#include "foc.h"
#include "pid.h"
#include "ad2s1210.h"

#define _PI 3.14159265358979323846f
#define _2PI (2.0f*3.14159265358979323846f)
#define _PI_2 1.57079632679489661923f
#define _3PI_2 (3.0f*_PI_2)
#define _SQRT3 1.732050807569f
#define _1DIV_SQRT3 0.57735026919f
// Square Root of 3
//#define _SQRT3	28  //1.73205081*16
#define RADIANS_TO_DEGREES(rad) ((rad)*180.0f/_PI)
#define DEGREES_TO_RADIANS(deg) ((deg)*_PI/180.0f)

static inline float fconstrain(float x, float min, float max)
{
    if(x<min)
        return min;
    else if(x>max)
        return max;
    else
        return x;
}

static inline float normalize_angle(float angle_rad)
{
    float const a = fmodf(angle_rad, _2PI);
    return a >= 0.0f ? a : (a + _2PI);
}

enum focState_e
{
    STOP,
    SIXSTEP,
    INTERPOLATION,
    RUNNING,
};
typedef volatile enum focState_e focState_t;

enum motormode_s
{
    AUTO,
    CALIBRATE_HALL,
    DISABLE_PWM,
    START,
};
typedef volatile enum motormode_s motormode_t;

enum controlmode_s
{
    VELOCITY_TORQUE,
    POSITION_VELOCITY_TORQUE,
};
typedef volatile enum controlmode_s controlmode_t;

enum motorsense_s
{
    HALL,
    ENCODER_SENSE,
    COLLECTOR,
};
typedef volatile enum motorsense_s motorsense_t;

struct picontrol_s
{
    int16_t       	gain_p;
	int16_t       	gain_i;
	int16_t       	limit_i;
	int16_t       	limit_output;
	int16_t       	recent_value;
	int32_t       	setpoint;
	int32_t       	integral_part;
	int16_t       	max_step;
	int32_t       	out;
	int8_t       	shift;
};
typedef volatile struct picontrol_s picontrol_t;

struct config_s
{
    uint32_t pwmFreq;  // Inverter PWM frequency
    float ts;           // tS - inv value (1/pwmFreq)
    uint32_t deadTime; // deadtime, 0-FF
    uint32_t sixStepThreshold;  // threshold for six step mode in timertics
    uint32_t recentTicsThreshold; // threshold for timertics since last event from tim8
    float phU_offset;
    float phV_offset;
    float phW_offset;
    uint8_t csvpwm;     // 0 - SPWM, 1 - CSVPWM
    int32_t current_sample_drop_rate;  // 3:250us cycle, 2:187us cycle, 1:125us cycle, 0: 62us cycle
    int32_t position_sample_drop_rate;
    float Hall_13;
    float Hall_32;
    float Hall_26;
    float Hall_64;
    float Hall_45;
    float Hall_51;
    float pole_pairs;
    float phase_offset_rad;
    float phase_synchro_offset_rad;
    uint8_t direction_inv; // 0: CCW-Normal, 1: CW-Reverse
    motormode_t motormode; 
    controlmode_t controlmode;
    motorsense_t motorsense;
    ad2s1210_resolution encoder_first_res;
    uint8_t goal_reverse;
    uint16_t sign_bit_velocity;
    float goal_torque_current_mA;
    float goal_flux_current_mA;
    float goal_velocity_rps;
    float goal_velocity_dps;
    float lsb_size_velocity_rps;
    float bits_to_radian_ratio;
    float max_radians;
    float min_position_deg;
    float max_position_deg;
    float goal_position_deg;
    float acceleration_dpss_max;
    float velocity_dps_max_ref;
    float pid_vel_kff;
    float pid_acc_kff;
    float max_current_mA;
    float max_velocity_rps;
};
typedef volatile struct config_s config_t;

#define CONFIG_DEFAULTS    \
    {                      \
        0, 0, 0            \
    }

struct data_s
{
    float phU_current;
    float phV_current;
    float phW_current;
    float phU_current_mA;
    float phV_current_mA;
    float phW_current_mA;
    float present_current_sq; //current squared
    float i_d;              //D-axis current 
    float i_q;              //Q-axis current 
    float u_d;              //measured D component of phase voltage
    float u_q;              //measured Q component of phase voltage
    q31_t u_abs;            //absolute value of U
    uint32_t tim8_recent;   //timertics since last event from TIM8
    uint32_t timertics;     //timertics between two hall events for 60Â° interpolation
    uint32_t timertics_for_velocity;
    q31_t rotorposition_absolute;
    int32_t current_samples;
    int32_t position_samples;
    float rotorposition_hall;   //angle when detect hall sensors 
    float theta_rad;
    float sine_theta;
    float cosine_theta;
    float velocity_dps;
    float velocity_rps;
    uint16_t hall_state;
    uint16_t hall_case;
    uint16_t hall_state_old;
    int8_t recent_rotor_direction;
    int8_t direction;       //for permanent reverse direction, 1 for normal direction, -1 for reverse
    uint16_t switchtime[3]; 
    uint32_t halfPwmPeriod;    // Variable: half of PWM period in timer tics
    uint16_t encoder_data_raw[2]; // [0] - position, [1] - velocity
    float reverse;
    float present_position_rad;
    float present_position_multi_rad;
    float last_position_rad;
    float delta_position_rad;
    float present_velocity_rps;
    float present_velocity_rad;
    int32_t present_revolution;

    float target_position_deg;
    float setpoint_position_deg;
    float remaining_distance_deg;
    float velocity_dps_max;
    float setpoint_acceleration_dpss;
    float velocity_feed_forward;
    float acceleration_feed_forward;
};
typedef volatile struct data_s data_t;

struct flags_s
{
    uint8_t angle_detect;
    uint8_t sixstep;
    uint8_t calib_hall;
    uint8_t uart;
};
typedef volatile struct flags_s flags_t;

struct filterData_s
{
    float in;       // Incomming: raw data[n]
    float inPr;     // Internal: raw data[n-1]
    float out;      // Output: filtered data[n]
    float T;        // Internal: filter time constant
};
typedef volatile struct filterData_s filterData_t;

struct foc_s
{
    focState_t focState;    // current system state
    config_t config;    // configuration
    data_t data;        // Internal variables   
    flags_t flags;
    pidReg_t pid_id;      // D-axis current controller
    pidReg_t pid_iq;      // Q-axis current controller
    pidReg_t pid_velocity;      // Velocity controller
    pidReg_t pid_position;
    filterData_t lpf_offsetIu;  // Low-pass filter for phase U current offset
    filterData_t lpf_offsetIv;  // Low-pass filter for phase V current offset
    filterData_t lpf_offsetIw;  // Low-pass filter for phase W current offset
};
typedef volatile struct foc_s foc_t;

static inline void LPF_calc(filterData_t *p)
{
    p->out = (p->T * (p->in - p->out)) + p->out;
}

void foc_init(foc_t *foc);
void foc_torque_update(foc_t *pf);
void clark_full_q31(q31_t Iu, q31_t Iv, q31_t Iw, q31_t *pIalpha, q31_t *pIbeta);
void startPIcontrol(foc_t *p);
q31_t PIcontrol(picontrol_t *pi_c);
void foc_Set_Flux_Angle(float setpoint_electrical_angle_rad, float setpoint_flux_voltage_V, foc_t *pf);
void foc_invClarkPark_PWMgen(float cos_theta, float sin_theta, foc_t *pf);
int foc_Calibrate(foc_t *pf);
float Position_Sensor_Get_Radians(); // !!! delete
void foc_test(foc_t *pf); // !!!
void foc_Set_Flux_Velocity(uint16_t present_time_us, float setpoint_electrical_velocity_dps, float setpoint_flux_voltage_V, foc_t *pf);
void foc_Set_Flux_Velocity_isr(float setpoint_electrical_velocity_dps, float setpoint_flux_voltage_V, foc_t *pf);
void calibrate_hall(foc_t *pf);

#endif