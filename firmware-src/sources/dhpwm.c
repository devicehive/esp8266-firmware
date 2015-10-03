/*
 * dhpwm.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include "dhpwm.h"
#include "dhgpio.h"
#include "dhdebug.h"
#include "user_config.h"

#define FRC1_ENABLE_TIMER  BIT7
#define FRC1_AUTO_LOAD  BIT6
typedef enum {
    DIVDED_BY_1 = 0,
    DIVDED_BY_16 = 4,
    DIVDED_BY_256 = 8
} TIMER_DIV_MODE;
typedef enum {
    TM_LEVEL_INT = 1,
    TM_EDGE_INT   = 0,
} TIMER_INT_MODE;

LOCAL uint32 mDisablePinOn[DHPWM_DEPTH + 1] = {0};
LOCAL unsigned int mPeriodUs = DHPWM_DEFAULT_PERIOD_US;
LOCAL unsigned char mCounter = 0;
LOCAL unsigned char mPwmInUse = 0;
LOCAL unsigned int mTotalCount = 0;
LOCAL uint32 mUsedPins = 0;

void ICACHE_FLASH_ATTR disarm_pwm_timer() {
	TM1_EDGE_INT_DISABLE();
	ETS_FRC1_INTR_DISABLE();
}

void on_timer() {
	if(mCounter == 0) {
		if(mTotalCount) {
			mTotalCount--;
			if(mTotalCount == 0) {
				disarm_pwm_timer();
				gpio_output_set(0, mDisablePinOn[DHPWM_DEPTH], mDisablePinOn[DHPWM_DEPTH], 0);
				dhpwm_disable_pins(mUsedPins | mDisablePinOn[0]);
				return;
			}
		} else {
			if(mPwmInUse == 0)
				disarm_pwm_timer();
			}
		gpio_output_set(mUsedPins, 0, mUsedPins, 0);
		mPwmInUse = 0;
	} else {
		const uint32 pins = mDisablePinOn[mCounter];
		if(pins) {
			gpio_output_set(0, pins, pins, 0);
			mPwmInUse = 1;
		}
	}
	mCounter++;
	if(mCounter >= DHPWM_DEPTH)
		mCounter = 0;
}

void ICACHE_FLASH_ATTR arm_pwm_timer() {
	// use mFrequency
	mPwmInUse = 1;
	mCounter = 1;
	unsigned int tacts;
	TIMER_DIV_MODE timer_div;
	if(mPeriodUs < 10000000) {
		timer_div = DIVDED_BY_1;
		tacts = mPeriodUs * (80 >> timer_div) / DHPWM_DEPTH;
	} else {
		timer_div = DIVDED_BY_256;
		tacts = mPeriodUs / DHPWM_DEPTH * 80 / (1 << timer_div);
	}
	RTC_REG_WRITE(FRC1_CTRL_ADDRESS, FRC1_AUTO_LOAD | timer_div | FRC1_ENABLE_TIMER | TM_EDGE_INT);
	ETS_FRC_TIMER1_INTR_ATTACH(on_timer, NULL);
	TM1_EDGE_INT_ENABLE();
	ETS_FRC1_INTR_ENABLE();
	RTC_REG_WRITE(FRC1_LOAD_ADDRESS, tacts);
	dhdebug("PWM enable with period %u us, timer %u/%u tacts", mPeriodUs, tacts, 1 << timer_div);
}

int ICACHE_FLASH_ATTR dhpwm_set_pwm(unsigned int *pinsduty, unsigned int pinsmask, unsigned int periodus, unsigned int count) {
	int i;
	if((pinsmask | DHGPIO_SUITABLE_PINS) != DHGPIO_SUITABLE_PINS || periodus < 500 || periodus >= 2000004000)
		return 0;
	for(i = 0; i <= DHGPIO_MAXGPIONUM; i++) {
		if(pinsduty[i] > DHPWM_DEPTH)
			return 0;
	}
	disarm_pwm_timer();
	mPeriodUs = periodus;
	mTotalCount = count;
	dhpwm_disable_pins(pinsmask);
	for(i = 0; i <= DHGPIO_MAXGPIONUM; i++) {
		if((pinsmask & (1 << i))) {
			dhdebug("PWM for %d pin, duty: %d", i, pinsduty[i]);
			if(pinsduty[i]) {
				mUsedPins |= (1 << i);
				mDisablePinOn[pinsduty[i]] |= (1 << i);
			} else {
				mDisablePinOn[0] |= (1 << i);
			}
		}
	}
	dhgpio_prepare_pins(mUsedPins | mDisablePinOn[0], 0);
	gpio_output_set(mUsedPins, mDisablePinOn[0], mUsedPins | mDisablePinOn[0], 0);
	if(mUsedPins)
		arm_pwm_timer();
	return 1;
}

unsigned int ICACHE_FLASH_ATTR dhpwm_get_period_us() {
	return mPeriodUs;
}

void ICACHE_FLASH_ATTR dhpwm_disable_pins(unsigned int pinsmask) {
	int i;
	for(i = 0; i < sizeof(mDisablePinOn)/sizeof(uint32); i++) {
		mDisablePinOn[i] &= ~pinsmask;
	}
	mUsedPins &= ~pinsmask;
}
