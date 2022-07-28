/*
 * \brief  Reset control driver for imx8
 * \author Shashank Anand <shashank.anand@gapfruit.com>
 * \date   2022-06-23
 */

/*
 * Copyright (C) 2013-2022 Genode Labs GmbH
 * Copyright (C) 2022 gapfruit AG
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Local include */
#include "reset_controller_driver.h"

void Reset_controller::Reset_controller_driver::reset()
{
	_mmio.write<Mmio::WDOG1_WCR>(Reset_controller::WCR_WDE | Reset_controller::WCR_WDT | Reset_controller::WCR_SRS);
}
