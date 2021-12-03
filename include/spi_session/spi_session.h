/*
 * \brief  SPI session client implementation
 * \author Jean-Adrien Domage <jean-adrien.domage@gapfruit.com>
 * \date   2021-04-28
 */

/*
 * Copyright (C) 2013-2021 Genode Labs GmbH
 * Copyright (C) 2021 gapfruit AG
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__SPI_SESSION__SPI_SESSION_H_
#define _INCLUDE__SPI_SESSION__SPI_SESSION_H_

#include <base/rpc.h>
#include <base/fixed_stdint.h>
#include <dataspace/capability.h>
#include <session/session.h>

namespace Spi {

	using namespace Genode;
	struct Settings;
	struct Transfer;
	class  Session;

	class Io_buffer_too_small : public Exception { };
	class Bus_error : public Exception { };

}

/**
 * Spi Transfer
 *
 * This struct is used for transferring data to the spi bus.
 */
struct Spi::Transfer
{
	/* buffer required for duplex_transfer and write operations */
	uint8_t const *tx_buffer      = nullptr;

	/* buffer required for duplex_transfer and read operations */
	uint8_t       *rx_buffer      = nullptr;

	/* any provided buffer must be at least buffer_size long */
	size_t         buffer_size    = 0;

	/* biggest burst size in bit */
	size_t         max_burst_size = 8;

	/* swap every bytes received on a burst level (driver take care of burst alignment) */
	bool           byte_reordering = true;
};

/**
 * Spi settings
 *
 * This struct is use to control the bus settings at a session level.
 */
struct Spi::Settings {

	enum {
		STATE_HIGH = 1,
		STATE_LOW  = 0,
	};

	/*
	 * Spi mode are as follow:
	 *  - MODE 0 (value 0): clk line POLARITY: 0 PHASE: 0
	 *  - MODE 1 (value 1): clk line POLARITY: 0 PHASE: 1
	 *  - MODE 2 (value 2): clk line POLARITY: 1 PHASE: 0
	 *  - MODE 3 (value 3): clk line POLARITY: 1 PHASE: 1
	 */
	uint32_t mode:                      2;

	/*
	 * Spi clock idle state control. This control if the clock must
	 * stay HIGH or stay LOW while it is idle
	 */
	uint32_t clock_idle_state:          1;

	/*
	 * Spi data lines state control. This control if the clock must
	 * stay HIGH or stay LOW while it is idle
	 */
	uint32_t data_lines_idle_state:     1;

	/*
	 * Spi slave select line active state, determinate which state has to be
	 * considered the active state.
	 */
	uint32_t ss_line_active_state:      1;
};


class Spi::Session: public Genode::Session
{
public:

	/*
	 * An spi session consume four capabilities, one for each tx/rx buffer,
	 * one dataspace capability for the server's session-object allocation, one for the session.
	 */
	enum { CAP_QUOTA = 4 };

	/**
	 * \return c string containing the service name
	 */
	static const char *service_name() { return "Spi"; }

	/**
	 * duplex_transfer
	 *
	 * Perform a full duplex data transfer to the endpoint slave device of
	 * the current session. A full duplex transfer consist of sending and receiving
	 * one bit on the same clock edge.
	 *
	 * \param     trxn - transfer struct containing relevant buffers
	 * \details   The client provide two buffers, rx_buffer and tx_buffer. tx_buffer
	 *            is used for reading byte that need to be transferred, rx_buffer
	 *            is used to store received bytes. Both buffer needs to be at least
	 *            buffer_size bytes long.
	 * \attention This methode is not thread-safe
	 */
	virtual void duplex_transfer(Spi::Transfer const&) = 0;

	/**
	 * \return session setting
	 */
	virtual Settings settings() = 0;

	/**
	 * \param setting - New setting to be applied to the session
	 */
	virtual void settings(Settings settings) = 0;

	GENODE_RPC_THROW(Rpc_duplex_transfer, void, bus_duplex_transfer,
	                 GENODE_TYPE_LIST(Io_buffer_too_small, Bus_error),
	                 size_t,
	                 size_t,
	                 bool);
	GENODE_RPC(Rpc_get_setting, Settings, settings);
	GENODE_RPC(Rpc_set_setting, void, settings, Settings);
	GENODE_RPC(Rpc_tx_dataspace, Dataspace_capability, tx_dataspace);
	GENODE_RPC(Rpc_rx_dataspace, Dataspace_capability, rx_dataspace);

	GENODE_RPC_INTERFACE(Rpc_duplex_transfer,
	                     Rpc_set_setting,
	                     Rpc_get_setting,
	                     Rpc_tx_dataspace,
	                     Rpc_rx_dataspace);
};

#endif // _INCLUDE__SPI_SESSION__SPI_SESSION_H_
