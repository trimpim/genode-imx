/*
 * \brief  System Reset Controller (reset_controller)'s mmio for imx8
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

#ifndef _INCLUDE__IMX8_RESET_CONTROLLER__MMIO_H_
#define _INCLUDE__IMX8_RESET_CONTROLLER__MMIO_H_


/* Genode include */
#include <platform_session/device.h>

/**
 * \attention Reset_controller stands for System Reset Controller
 */
namespace Reset_controller {
	using namespace Genode;
    
	struct Mmio;
}


/**
 * \attention Keep in mind that on ARM a word usually refer to 4 bytes.
 */
struct Reset_controller::Mmio : Platform::Device::Mmio
{
	/**
	 * \attention Data registers must be accessed via word operation.
	 */
	struct WDOG1_WCR : Mmio::Register<0x00, 16> { };

	explicit Mmio(Platform::Device &device) : Platform::Device::Mmio { device } { }

};

#endif /* _INCLUDE_IMX8_RESET_CONTROLLER__MMIO_H_ */
