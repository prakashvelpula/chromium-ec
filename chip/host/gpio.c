/* Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* GPIO module for emulator */

#include "common.h"
#include "gpio.h"

test_mockable void gpio_pre_init(void)
{
	/* Nothing */
}

test_mockable int gpio_get_level(enum gpio_signal signal)
{
	return 0;
}

test_mockable void gpio_set_level(enum gpio_signal signal, int value)
{
	/* Nothing */
}