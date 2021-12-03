/*
 * \brief  ECSPI driver base class to be implemented by platform specific 
 * \author Jean-Adrien Domage <jean-adrien.domage@gapfruit.com>
 * \date   2021-04-20
 */

/*
 * Copyright (C) 2013-2021 Genode Labs GmbH
 * Copyright (C) 2021 gapfruit AG
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__SPI_DRIVER__INTERFACE_H_
#define _INCLUDE__SPI_DRIVER__INTERFACE_H_

/* Genode includes */
#include <base/env.h>
#include <util/xml_node.h>

/* Os include */
#include <spi_session/spi_session.h>

namespace Spi {

	using namespace Genode;
	class Driver;

	Driver* initialize(Genode::Env &env, Xml_node const &config);
}

/*
 * Base class for platform specific driver to implement
 *
 * Note about the endianness: the driver is transparent.
 *
 * The driver read/write bytes to memory in the order they are
 * read/write to the bus.
 * It is the responsibility of the component interacting with
 * a slave device on the bus to figure out how to interpret the data. 
 */
struct Spi::Driver : Interface
{

	/* Spi bus transaction */
	struct Bus_transaction {
		Settings       settings        = { }; /* Client session setting */
		size_t         slave_select    = 0;   /* Slave select line. */
		size_t         max_burst_size  = 8;
		bool           byte_reordering = true;
		uint8_t const *tx_buffer       = nullptr;
		uint8_t       *rx_buffer       = nullptr;
		size_t         tx_offset       = 0; /* used by driver implementation */
		size_t         rx_offset       = 0; /* used by driver implementation */
		size_t         buffer_size     = 0;
	};


	/*
	 * Execute an Spi transaction
	 *
	 * \details The data provided threw the transaction buffer is read to transmit data. The received data are
	 * directly copied into that same buffer.
	 * \attention This methode is NOT thread safe
	 *
	 * \param trxn transaction to execute
	 * \return the received bytes count
	 */
	virtual void transfer(Bus_transaction& trxn) = 0;


	/*
	 * Current driver implementation name getter
	 *
	 * \return Driver name string
	 *
	 * Details this method is overridable to customise the name based on the platform.
	 */
	virtual char const *name() const { return "SPI INTERFACE"; }
};

#endif /* _INCLUDE__SPI_DRIVER__INTERFACE_H_ */
