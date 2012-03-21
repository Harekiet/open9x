/*
 * Authors - Gerard Valade <gerard.valade@gmail.com>
 *
 * Adapted from mavlink for ardupilot
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

#ifndef _MAVLINK_H_
#define _MAVLINK_H_

#define MAVLINK_USE_CONVENIENCE_FUNCTIONS
#define MAVLINK_COMM_NUM_BUFFERS 1

//#define MAVLINK10

#ifdef MAVLINK10
#include "GCS_MAVLink/include_v1.0/mavlink_types.h"
#else
#include "GCS_MAVLink/include/mavlink_types.h"
#endif

#include "serial.h"

//#include "include/mavlink_helpers.h"

extern mavlink_system_t mavlink_system;

extern void SERIAL_start_uart_send();
extern void SERIAL_end_uart_send();
extern void SERIAL_send_uart_bytes(uint8_t * buf, uint16_t len);

#define MAVLINK_START_UART_SEND(chan,len) SERIAL_start_uart_send()
#define MAVLINK_END_UART_SEND(chan,len) SERIAL_end_uart_send()
#define MAVLINK_SEND_UART_BYTES(chan,buf,len) SERIAL_send_uart_bytes(buf,len)

#ifdef MAVLINK10
#include "GCS_MAVLink/include_v1.0/ardupilotmega/mavlink.h"
#else
#include "GCS_MAVLink/include/ardupilotmega/mavlink.h"
#endif

#define MAVLINK_PARAMS

#define ERROR_NUM_MODES 99
#define ERROR_MAV_ACTION_NB 99

#ifdef MAVLINK_PARAMS

enum ACM_PARAMS {
	RATE_YAW_P, // Rate Yaw
	RATE_YAW_I, // Rate Yaw
	STB_YAW_P, // Stabilize Yaw
	STB_YAW_I, // Stabilize Yaw
	RATE_PIT_P, // Rate Pitch
	RATE_PIT_I, // Rate Pitch
	STB_PIT_P, // Stabilize Pitch
	STB_PIT_I, // Stabilize Pitch
	RATE_RLL_P, // Rate Roll
	RATE_RLL_I, // Rate Roll
	STB_RLL_P, // Stabilize Roll
	STB_RLL_I, // Stabilize Roll
	THR_ALT_P, // THR_BAR, // Altitude Hold
	THR_ALT_I, // THR_BAR, // Altitude Hold
	HLD_LON_P, // Loiter
	HLD_LON_I, // Loiter
	HLD_LAT_P, // Loiter
	HLD_LAT_I, // Loiter
	NAV_LON_P, // Nav WP
	NAV_LON_I, // Nav WP
	NAV_LAT_P, // Nav WP
	NAV_LAT_I, // Nav WP
	NB_PID_PARAMS, // Number of PI Parameters
	LOW_VOLT = NB_PID_PARAMS,
	IN_VOLT, //
	BATT_MONITOR, //
	BATT_CAPACITY, //
	NB_PARAMS
};
//#define NB_PID_PARAMS 24
#define NB_COL_PARAMS 2
#define NB_ROW_PARAMS ((NB_PARAMS+1)/NB_COL_PARAMS)

typedef struct MavlinkParam_ {
	uint8_t repeat :4;
	uint8_t valid :4;
	float value;
} MavlinkParam_t;

#endif

typedef struct Location_ {
	float lat; ///< Latitude in degrees
	float lon; ///< Longitude in degrees
	float alt; ///< Altitude in meters
} Location_t;

typedef struct Telemetry_Data_ {
	// INFOS
	uint8_t status; ///< System status flag, see MAV_STATUS ENUM
	uint16_t packet_drop;
	//uint8_t mode;
	//uint8_t nav_mode;
	uint8_t rcv_control_mode; ///< System mode, see MAV_MODE ENUM in mavlink/include/mavlink_types.h
	//uint16_t load; ///< Maximum usage in percent of the mainloop time, (0%: 0, 100%: 1000) should be always below 1000
	uint8_t vbat; ///< Battery voltage, in millivolts (1 = 1 millivolt)
	uint8_t vbat_low;

	// MSG ACTION / ACK
	uint8_t req_mode;
	int8_t ack_result;

	// GPS
	uint8_t fix_type; ///< 0-1: no fix, 2: 2D fix, 3: 3D fix. Some applications will not use the value of this field unless it is at least two, so always correctly fill in the fix.
	uint8_t satellites_visible; ///< Number of satellites visible
	Location_t loc_current;
	float eph;
	float hdg;
	float v; // Ground speed

#ifdef MAVLINK_PARAMS
	// Params
	MavlinkParam_t params[NB_PARAMS];
#endif

} Telemetry_Data_t;

// Telemetry data hold
extern Telemetry_Data_t telemetry_data;

#ifndef MAVLINK10
extern inline uint8_t MAVLINK_NavMode2CtrlMode(uint8_t mode, uint8_t nav_mode) {

	uint8_t control_mode = ERROR_NUM_MODES;
	switch (mode) {
	case MAV_MODE_UNINIT:
		control_mode = INITIALISING;
		break;

	case MAV_MODE_AUTO:
		switch (nav_mode) {
		case MAV_NAV_HOLD: // ACM
		case MAV_NAV_LOITER: // APM
			control_mode = LOITER;
			break;
		case MAV_NAV_WAYPOINT:
			control_mode = AUTO;
			break;
		case MAV_NAV_RETURNING:
			control_mode = RTL;
			break;
		}
		break;
	case MAV_MODE_GUIDED:
		control_mode = GUIDED;
		break;

		/* from ardupilot */
	case MAV_MODE_MANUAL:
		control_mode = MANUAL;
		break;
	case MAV_MODE_TEST1:
		control_mode = STABILIZE;
		break;
	case MAV_MODE_TEST2:
		switch (nav_mode) {
		case 1:
			control_mode = FLY_BY_WIRE_A;
			break;
		case 2:
			control_mode = FLY_BY_WIRE_B;
			break;
		}
		break;
	case MAV_MODE_TEST3:
		control_mode = CIRCLE;
		break;

	default:
		if (mode >= 100) {
			control_mode = mode - 100;
		}
		break;
	}
	return control_mode;
}
#endif

extern inline uint8_t MAVLINK_CtrlMode2Action(uint8_t mode) {
	uint8_t action;
	switch (mode) {
	case STABILIZE:
		action = MAV_ACTION_SET_MANUAL;
		break;
	case RTL:
		action = MAV_ACTION_RETURN;
		break;
	case LAND:
		action = MAV_ACTION_LAND;
		break;
	case LOITER:
		action = MAV_ACTION_LOITER;
		break;
	case AUTO:
		action = MAV_ACTION_SET_AUTO;
		break;
	case MANUAL:
		action = MAV_ACTION_SET_MANUAL;
		break;
	default:
		action = ERROR_MAV_ACTION_NB;
		break;
	}
	return action;
}
#if 0
extern inline uint8_t MAVLINK_Action2CtrlMode(uint8_t action) {
	uint8_t mode = ERROR_NUM_MODES;
	switch (action) {
	case MAV_ACTION_SET_MANUAL:
		mode = STABILIZE;
		break;
		/*
		 case ACRO:
		 action = 0;
		 break;
		 case SIMPLE:
		 action = 0;
		 break;
		 case ALT_HOLD:
		 action = 0;
		 break;*/
	case MAV_ACTION_SET_AUTO:
		mode = AUTO;
		break;
		/*case GUIDED:
		 action = 0;
		 break;*/
	case MAV_ACTION_LOITER:
		mode = LOITER;
		break;
	case MAV_ACTION_RETURN:
		mode = RTL;
		break;
		/*case CIRCLE:
		 action = 0;
		 break;*/
	default:
		break;
	}
	return action;
}
#endif
void check_mavlink();
void MAVLINK_Init(void);
void menuProcMavlink(uint8_t event);
void MAVLINK10mspoll(uint16_t time);

#ifdef MAVLINK_PARAMS

void putsMavlinParams(uint8_t x, uint8_t y, uint8_t idx, uint8_t att);
void setMavlinParamsValue(uint8_t idx, float val);

inline uint8_t getIdxParam(uint8_t rowIdx, uint8_t colIdx) {
	return (rowIdx * NB_COL_PARAMS) + colIdx;
}

inline MavlinkParam_t * getParam(uint8_t idx) {
	return &telemetry_data.params[idx];
}

inline float getMavlinParamsValue(uint8_t idx) {
	return telemetry_data.params[idx].value;
}

inline uint8_t isDirtyParamsValue(uint8_t idx) {
	return telemetry_data.params[idx].repeat;
}

inline uint8_t isValidParamsValue(uint8_t idx) {
	return telemetry_data.params[idx].valid;
}

inline float getCoefPrecis(uint8_t precis) {
	switch (precis) {
	case 1:
		return 10.0;
	case 2:
		return 100.0;
	case 3:
		return 1000.0;
	}
	return 1.0;

}

inline int16_t getMaxMavlinParamsValue(uint8_t idx) {
	int16_t max = 0;
	switch (idx) {
	case LOW_VOLT:
		max = 2500; // 25.0 Volt max
		break;
	case IN_VOLT:
		max = 900; // 7.00 Volt max
		break;
	case BATT_MONITOR:
		max = 3;
		break;
	case BATT_CAPACITY:
		max = 7000; // 7000 mAh max
		break;
	default:
		if (idx < NB_PID_PARAMS) {
			max = (idx & 0x01) ? 1000 : 750;
		}
		break;
	}
	return max;
}

inline uint8_t getPrecisMavlinParamsValue(uint8_t idx) {
	uint8_t precis = 2;
	switch (idx) {
	case LOW_VOLT:
		precis = 2;
		break;
	case IN_VOLT:
		precis = 2;
		break;
	case BATT_MONITOR:
		precis = 0;
		break;
	case BATT_CAPACITY:
		precis = 0;
		break;
	default:
		if (idx < NB_PID_PARAMS) {
			if (idx & 0x01)
				precis = 3;
		}
		break;
	}
	return precis;
}

void lcd_outdezFloat(uint8_t x, uint8_t y, float val, uint8_t precis, uint8_t mode);
#endif

#endif

