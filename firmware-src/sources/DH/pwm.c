/**
 * @file
 * @brief Software PWM implementation for ESP8266 firmware.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "DH/pwm.h"
#include "DH/adc.h"
#include "dhdebug.h"

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include <ets_forward.h>

#define FRC1_ENABLE_TIMER  BIT7
#define FRC1_AUTO_LOAD     BIT6

#define MIN_PERIOD_US 500
#define MAX_PERIOD_US 2000004000


typedef enum {
	DIVDED_BY_1   = 0,
	DIVDED_BY_16  = 4,
	DIVDED_BY_256 = 8
} TIMER_DIV_MODE;

typedef enum {
	TM_LEVEL_INT = 1,
	TM_EDGE_INT  = 0
} TIMER_INT_MODE;

// module variables
static DHGpioPinMask mDisablePinOn[DH_PWM_DEPTH + 1] = {0};
static unsigned int mPeriodUs = DH_PWM_DEFAULT_PERIOD_US;
static unsigned char mCounter = 0;
static unsigned char mPwmInUse = 0;
static unsigned int mTotalCount = 0;
static DHGpioPinMask mUsedPins = 0;


/**
 * @brief Disable PWM timer.
 */
static void ICACHE_FLASH_ATTR disarm_timer(void)
{
	TM1_EDGE_INT_DISABLE();
	ETS_FRC1_INTR_DISABLE();
}

/**
 * @brief Timer callback function.
 */
static void timer_cb(void)
{
	if (mCounter == 0) {
		if (mTotalCount) {
			mTotalCount--;
			if (mTotalCount == 0) {
				disarm_timer();
				gpio_output_set(0, mDisablePinOn[DH_PWM_DEPTH],
				                mDisablePinOn[DH_PWM_DEPTH], 0);
				dh_pwm_disable(mUsedPins | mDisablePinOn[0]);
				return;
			}
		} else {
			if (mPwmInUse == 0)
				disarm_timer();
			}
		gpio_output_set(mUsedPins, 0, mUsedPins, 0);
		mPwmInUse = 0;
	} else {
		const DHGpioPinMask pins = mDisablePinOn[mCounter];
		if (pins) {
			gpio_output_set(0, pins, pins, 0);
			mPwmInUse = 1;
		}
	}
	mCounter++;
	if (mCounter >= DH_PWM_DEPTH)
		mCounter = 0;
}


/**
 * @brief Enable PWM timer.
 */
static void ICACHE_FLASH_ATTR arm_timer(void)
{
	// use mFrequency
	mPwmInUse = 1;
	mCounter = 1;
	unsigned int tacts;
	TIMER_DIV_MODE timer_div;
	if (mPeriodUs < 10000000) {
		timer_div = DIVDED_BY_1;
		tacts = mPeriodUs * (80 >> timer_div) / DH_PWM_DEPTH;
	} else {
		timer_div = DIVDED_BY_256;
		tacts = mPeriodUs / DH_PWM_DEPTH * 80 / (1 << timer_div);
	}

	RTC_REG_WRITE(FRC1_CTRL_ADDRESS, FRC1_AUTO_LOAD | timer_div | FRC1_ENABLE_TIMER | TM_EDGE_INT);
	ETS_FRC_TIMER1_INTR_ATTACH((ets_isr_t)timer_cb, NULL);
	TM1_EDGE_INT_ENABLE();
	ETS_FRC1_INTR_ENABLE();
	RTC_REG_WRITE(FRC1_LOAD_ADDRESS, tacts);

	dhdebug("PWM enable with period %u us, timer: %u/%u tacts",
	        mPeriodUs, tacts, 1 << timer_div);
}


/*
 * dh_pwm_start() implementation.
 */
int ICACHE_FLASH_ATTR dh_pwm_start(uint32_t duty[DH_GPIO_PIN_COUNT],
                                   DHGpioPinMask pins,
                                   unsigned int period_us,
                                   unsigned int count)
{
	if (pins & ~DH_GPIO_SUITABLE_PINS)
		return -1; // unsuitable pins
	if (period_us < MIN_PERIOD_US || period_us >= MAX_PERIOD_US)
		return -2; // period is out of range
	int i;
	for (i = 0; i < DH_GPIO_PIN_COUNT; i++) {
		if (duty[i] > DH_PWM_DEPTH)
			return -3; // bad duty value
	}

	disarm_timer();
	mPeriodUs = period_us;
	mTotalCount = count;
	dh_pwm_disable(pins);
	for (i = 0; i < DH_GPIO_PIN_COUNT; i++) {
		const DHGpioPinMask pin = DH_GPIO_PIN(i);
		if ((pins & pin)) {
			dhdebug("PWM for %d pin, duty: %d", i, duty[i]);
			if (duty[i]) {
				mUsedPins |= pin;
				mDisablePinOn[duty[i]] |= pin;
			} else {
				mDisablePinOn[0] |= pin;
			}
		}
	}

	dh_gpio_prepare_pins(mUsedPins | mDisablePinOn[0], 0);
	gpio_output_set(mUsedPins, mDisablePinOn[0],
	                mUsedPins | mDisablePinOn[0], 0);
	if (mUsedPins)
		arm_timer();

	return 0; // OK
}


/*
 * dh_pwm_get_period_us() implementation.
 */
unsigned int ICACHE_FLASH_ATTR dh_pwm_get_period_us(void)
{
	return mPeriodUs;
}


/*
 * dh_pwm_disable() implementation.
 */
void ICACHE_FLASH_ATTR dh_pwm_disable(DHGpioPinMask pins)
{
	int i, n = sizeof(mDisablePinOn)/sizeof(mDisablePinOn[0]);
	for (i = 0; i < n; i++) {
		mDisablePinOn[i] &= ~pins;
	}
	mUsedPins &= ~pins;
}
