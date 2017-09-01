/*******************************************************************************
* Copyright (C) 2014 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/

/* $Revision: 3040 $ $Date: 2014-08-22 16:05:40 -0600 (Fri, 22 Aug 2014) $ */

#ifndef _POWER_H
#define _POWER_H

#include "pwrman_regs.h"
#include "pwrseq_regs.h"

/**
 * @file  power.h
 * @addtogroup pwrman Power Management
 * @{
 * @brief This is the high level API for the power management module
 *        of the MAX32600 family of ARM Cortex based embedded microcontrollers.
 */

/**
 * @brief Enables all GPIOs.
 */
void PWR_EnableGPIO(void);

/**
 * @brief Disables all GPIOs.
 */
void PWR_DisableGPIO(void);

/**
 * @brief Sets up WUD for designated GPIO port and pin.
 *
 * @param port      Port index.
 * @param pin       Pin index.
 * @param act_high  1 for active high, 0 for active low.
 */
void PWR_SetGPIOWUD(uint8_t port, uint8_t pin, uint8_t act_high);

/**
 * @brief Clears WUD for all GPIO ports and pins.
 */
void PWR_ClearAllGPIOWUD(void);

/**
 * @brief Clears WUD for designated GPIO port and pin.
 *
 * @param port      Port index.
 * @param pin       Pin index.
 */
void PWR_ClearGPIOWUD(uint8_t port, uint8_t pin);

/**
 * @brief Sets up WUD for designated comparator.
 *
 * @param index         Comparator index.
 * @param rising_edge   1 for rising edge, 0 for falling edge.
 */
void PWR_SetCompWUD(uint8_t index, uint8_t rising_edge);

/**
 * @brief Clears WUD for designated comparator.
 *
 * @param index   Comparator index.
 */
void PWR_ClearCompWUD(uint8_t index);

/**
 * @brief Clears WUD for all comparators.
 *
 */
void PWR_ClearAllCompWUD(void);

/**
 * @brief Clears power sequencer flag latch.
 */
void PWR_ClearFlags(void);

/**
 * @brief Defines power modes.
 */
typedef enum {
    /** ARM deep sleep mode without data retention (WFE) */
    MXC_E_PWR_MODE_LP0 = 0,
    /** ARM deep sleep mode with data retention (WFE) */
    MXC_E_PWR_MODE_LP1,
    /** ARM sleep mode (WFI) */
    MXC_E_PWR_MODE_LP2,
    /** No sleep mode */
    MXC_E_PWR_MODE_LP3
} mxc_pwr_mode_t;

/**
 * @brief Sets processor mode.
 *
 * @param mode    Sleep mode.
 * @param wakeup  Callback function for return from LP1.
 */
void PWR_SetMode(mxc_pwr_mode_t mode, void (*wakeup)(void));

/**
 * @brief Sends processor to sleep. Chooses WFI/WFE based on current mode.
 */
void PWR_Sleep(void);

/**
 * @brief Initializes the power management module.
 */
void PWR_Init(void);

/**
 * @brief Defines modules to enable/disable.
 */
typedef enum {
    /** AFE Powered */
    MXC_E_PWR_ENABLE_AFE = 0,
    /** I/O Active */
    MXC_E_PWR_ENABLE_IO,
    /** USB Powered */
    MXC_E_PWR_ENABLE_USB,
    /** Static Pullups Enabled */
    MXC_E_PWR_ENABLE_STATIC_PULLUPS
} mxc_pwr_enable_t;

/**
 * @brief Enables modules in pwr_rst_ctrl.
 */
void PWR_Enable(mxc_pwr_enable_t module);

/**
 * @brief Disables modules in pwr_rst_ctrl.
 */
void PWR_Disable(mxc_pwr_enable_t module);

typedef enum {
    /** Trickle charger with no diode and 250 ohm resistor */
    MXC_E_PWR_TRICKLE_CHARGER_NO_DIODE_W_250_OHM = 0x5,
    /** Trickle charger with no diode and 2k ohm resistor */
    MXC_E_PWR_TRICKLE_CHARGER_NO_DIODE_W_2K_OHM = 0x6,
    /** Trickle charger with no diode and 4k ohm resistor */
    MXC_E_PWR_TRICKLE_CHARGER_NO_DIODE_W_4K_OHM = 0x7,
    /** Trickle charger with diode and 250 ohm resistor */
    MXC_E_PWR_TRICKLE_CHARGER_DIODE_W_250_OHM = 0x9,
    /** Trickle charger with diode and 2k ohm resistor */
    MXC_E_PWR_TRICKLE_CHARGER_DIODE_W_2K_OHM = 0xA,
    /** Trickle charger with diode and 4k ohm resistor */
    MXC_E_PWR_TRICKLE_CHARGER_DIODE_W_4K_OHM = 0xB,
} mxc_pwr_trickle_charger_t;

/**
 * @brief Set up trickle charger super-cap
 *
 * @param decode     Trickle charger resistor and diode.
 */
void PWR_SetTrickleCharger(mxc_pwr_trickle_charger_t decode);

/**
 * @brief Defines devices to enable/disable.
 */
typedef enum {
    /** LDO regulator power switch */
    MXC_E_PWR_DEVICE_LDO  = 3,
    /** CHZY regulator power switch */
    MXC_E_PWR_DEVICE_CHZY = 5,
    /** Relaxation oscillator power switch */
    MXC_E_PWR_DEVICE_RO   = 7,
    /** Nano ring oscillator power switch */
    MXC_E_PWR_DEVICE_NR   = 9,
    /** Real-time clock power switch */
    MXC_E_PWR_DEVICE_RTC  = 11,
    /** VDD3 system voltage monitor power switch */
    MXC_E_PWR_DEVICE_SVM3 = 13,
    /** VREG18 system voltage monitor power switch */
    MXC_E_PWR_DEVICE_SVM1 = 15,
    /** VRTC system voltage monitor power switch */
    MXC_E_PWR_DEVICE_SVMRTC = 17,
    /** VDDA3 system voltage monitor power switch */
    MXC_E_PWR_DEVICE_SVMVDDA3 = 19,
} mxc_pwr_device_t;

/**
 * @brief Enable a module in run mode.
 *
 * @param device    Device to enable in run mode.
 */
void PWR_EnableDevRun(mxc_pwr_device_t device);

/**
 * @brief Enable a module in sleep mode.
 *
 * @param device    Device to enable in sleep mode.
 */
void PWR_EnableDevSleep(mxc_pwr_device_t device);

/**
 * @brief Disable a module in run mode.
 *
 * @param device    Device to disable in run mode.
 */
void PWR_DisableDevRun(mxc_pwr_device_t device);

/**
 * @brief Disable a module in sleep mode.
 *
 * @param device    Device to disable in sleep mode.
 */
void PWR_DisableDevSleep(mxc_pwr_device_t device);

/**
 * @brief Enable all wakeup events to wake up power sequencer.
 */
void PWR_EnableAllWakeupEvents(void);

/**
 * @brief Disable all wakeup events to wake up power sequencer.
 */
void PWR_DisableAllWakeupEvents(void);

/**
 * @brief Defines event masks to enable/disable.
 */
typedef enum {
    /** Firmware reset event */
    MXC_E_PWR_EVENT_SYS_REBOOT = 1,
    /** Power fail event */
    MXC_E_PWR_EVENT_POWER_FAIL,
    /** Boot fail event */
    MXC_E_PWR_EVENT_BOOT_FAIL,
    /** Comparator wakeup event */
    MXC_E_PWR_EVENT_COMP_FLAG,
    /** GPIO wakeup event */
    MXC_E_PWR_EVENT_IO_FLAG,
    /** VDD3 reset comparator event */
    MXC_E_PWR_EVENT_VDD3_RST,
    /** VDD3 warning comparator event */
    MXC_E_PWR_EVENT_VDD3_WARN,
    /** VREG18 reset comparator event*/
    MXC_E_PWR_EVENT_VDD1_RST,
    /** VREG18 reset low comparator event */
    MXC_E_PWR_EVENT_VDD1_LOW_RST,
    /** VREG18 warning comparator event */
    MXC_E_PWR_EVENT_VDD1_WARN,
    /** VRTC comparator event */
    MXC_E_PWR_EVENT_VRTC_WARN,
    /** POR3 and POR3_lite event */
    MXC_E_PWR_EVENT_POR3Z_FAIL,
    /** RTC cmpr0 event */
    MXC_E_PWR_EVENT_RTC_CMPR0,
    /** RTC cmpr1 event */
    MXC_E_PWR_EVENT_RTC_CMPR1,
    /** RTC prescale event */
    MXC_E_PWR_EVENT_RTC_PRESCALE_CMP,
    /** RTC rollover event */
    MXC_E_PWR_EVENT_RTC_ROLLOVER,
    /** RTC brownout event */
    MXC_E_PWR_EVENT_BROWNOUT,
    /** RTC usb plug inserted event*/
    MXC_E_PWR_EVENT_USB_PLUG,
    /** RTC usb plug removed event */
    MXC_E_PWR_EVENT_USB_REMOVE,
    /** VDD22 reset comparator event */
    MXC_E_PWR_EVENT_VDD22_RST,
    /** VDD195 reset comparator event */
    MXC_E_PWR_EVENT_VDD195_RST
} mxc_pwr_event_t;

/**
 * @brief Enables wakeup events to wake up power sequencer for a given event.
 *
 * @param event     Event mask to enable.
 */
void PWR_EnableWakeupEvent(mxc_pwr_event_t event);

/**
 * @brief Disables wakeup event to wake up power sequencer for a given event.
 *
 * @param event     Event mask to disable.
 */
void PWR_DisableWakeupEvent(mxc_pwr_event_t event);

/**
 * @}
 */

#endif /* POWER_H_ */
