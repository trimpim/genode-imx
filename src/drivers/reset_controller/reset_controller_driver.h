/*
 * \brief  Reset_controller driver for imx8
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

#ifndef _INCLUDE__IMX8_RESET_CONTROLLER__DRIVER_H_
#define _INCLUDE__IMX8_RESET_CONTROLLER__DRIVER_H_

/* Genode includes */
#include <base/env.h>
#include <base/semaphore.h>
#include <platform_session/connection.h>

/* Local includes */
#include "reset_controller_mmio.h"

namespace Reset_controller
{
	using namespace Genode;

	constexpr unsigned WCR_WDE = 0x04;
	constexpr unsigned WCR_WDT = 0x08;
	constexpr unsigned WCR_SRS = 0x10;

	class Reset_controller_driver;
}

class Reset_controller::Reset_controller_driver
{
	private:
		Genode::Env &_env;

		/* platform connection */
		Platform::Connection   _platform { _env };
		Platform::Device       _device   { _platform };

		/* iomem */
		Reset_controller::Mmio _mmio     { _device };

	public:
		Reset_controller_driver(Genode::Env &env) : _env { env }
		{
			Genode::log("Init Reset_controller driver.");
		}

		/* performs a POR reset of all four cores */
		void reset();
};

#endif /* _INCLUDE__IMX8_RESET_CONTROLLER__DRIVER_H_ */
