/*
 *  \brief  VFS plugin interface implementation
 *  \author Alice Domage
 *  \author Pirmin Duss
 *  \date   2022-06-28
 */

/*
 * Copyright (C) 2024 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/mutex.h>
#include <irq_session/client.h>
#include <platform_session/connection.h>
#include <timer_session/connection.h>
#include <platform_session/device.h>

/* i2c includes */
#include <i2c/driver_base.h>

/* local includes */
#include "mmio.h"


namespace I2c_imx
{
	using namespace Genode;

	class Driver;
}


namespace {

	Genode::uint8_t _bus_speed_to_divider(Genode::uint16_t bus_speed_khz)
	{
	   /* the table can be found:
	    * IMX8MMRM.pdf on 5233
	    *
	    * The bus base frequency is 25MHz.
	    */

	   if (bus_speed_khz >= 400) return 0x2a;   /* divide by   64 maximal speed supported */
	   if (bus_speed_khz >= 200) return 0x2f;   /* divide by  128 */
	   if (bus_speed_khz >= 100) return 0x33;   /* divide by  256 */
	   if (bus_speed_khz >=  50) return 0x37;   /* divide by  512 */
	   if (bus_speed_khz >=  25) return 0x3B;   /* divide by 1024 */

	   return 0x3F;  /* divide by 2048 minimal speed */
	}
}


class I2c_imx::Driver final : public I2c::Driver_base
{
	private:

		Env   &_env;

		Platform::Connection      _platform         { _env };
		Platform::Device          _device           { _platform };
		I2c::Mmio                 _mmio             { _device };
		Platform::Device::Irq     _irq              { _device };
		Io_signal_handler<Driver> _irq_handler      { _env.ep(), *this, &Driver::_irq_handle };
		Timer::Connection         _timer            { _env };

		unsigned                  _sem_cnt          { 1 };
		uint16_t                  _bus_speed_khz;
		bool                      _verbose;

		void _bus_reset();
		void _bus_start();
		void _bus_stop();
		void _bus_write(Genode::uint8_t data);
		void _bus_busy();

		void _wait_for_irq();
		void _irq_handle() { _sem_cnt = 0; }

		void _write(I2c::Bus_address &, I2c::Message&);
		void _read(I2c::Bus_address &, I2c::Message&);

	public:

		Driver(Env &env, uint16_t bus_speed_khz, bool verbose) :
			Driver_base     { },
			_env            { env },
			_bus_speed_khz  { bus_speed_khz },
			_verbose        { verbose }
		{ }

		void transfer(I2c::Bus_address &a, I2c::Transaction &t) override;
};


void I2c_imx::Driver::_wait_for_irq()
{
	using I2c::Mmio;

	_sem_cnt++;
	while (_sem_cnt > 0)
		_env.ep().wait_and_dispatch_one_io_signal();

	if (_mmio.read<Mmio::Control::Master_slave_select>() == 0) {
		_bus_stop();
		if (_verbose) {
			error("Arbitration lost on bus ");
		}
		throw I2c::Bus_error();
	}
}


void I2c_imx::Driver::_bus_busy()
{
	using I2c::Mmio;

	uint64_t const start_time = _timer.elapsed_ms();
	while (!_mmio.read<Mmio::Status::Busy>()) {
		uint64_t const current = _timer.elapsed_ms();
		if (current - start_time > 1000) {
			_bus_stop();
			if (_verbose) {
				error("Timeout on bus");
			}
			throw I2c::Bus_error();
		}
	}
}


void I2c_imx::Driver::_bus_reset()
{
	using I2c::Mmio;

	_mmio.write<Mmio::Control>(0);
	_mmio.write<Mmio::Status>(0);
}


void I2c_imx::Driver::_bus_start()
{
	using I2c::Mmio;

	/* input root 90 is 25Mhz select divisor to approximate desired bus speed */
	_mmio.write<Mmio::Freq_divider>(_bus_speed_to_divider(_bus_speed_khz));
	_mmio.write<Mmio::Status>(0);
	_mmio.write<Mmio::Control>(Mmio::Control::Enable::bits(1));

	uint64_t const start_time = _timer.elapsed_ms();
	while (!_mmio.read<Mmio::Control::Enable>()) {
		uint64_t const current = _timer.elapsed_ms();
		if (current - start_time > 1000) {
			_bus_stop();
			if (_verbose) {
				error("Timeout on bus");
			}
			throw I2c::Bus_error();
		}
	}

	_mmio.write<Mmio::Control::Master_slave_select>(1);

	_bus_busy();

	_mmio.write<Mmio::Control>(Mmio::Control::Tx_rx_select::bits(1)        |
	                           Mmio::Control::Tx_ack_enable::bits(1)       |
	                           Mmio::Control::Irq_enable::bits(1)          |
	                           Mmio::Control::Master_slave_select::bits(1) |
	                           Mmio::Control::Enable::bits(1));

	_mmio.write<Mmio::Status::Ial>(0);
}


void I2c_imx::Driver::_bus_stop()
{
	using I2c::Mmio;

	_mmio.write<Mmio::Control>(0);
}


void I2c_imx::Driver::_bus_write(uint8_t data)
{
	using I2c::Mmio;

	_mmio.write<Mmio::Data>(data);

	do { _wait_for_irq(); }
	while (!_mmio.read<Mmio::Status::Irq>());

	_mmio.write<Mmio::Status::Irq>(0);
	_irq.ack();

	if (_mmio.read<Mmio::Status::Rcv_ack>()) {
		_bus_stop();
		if (_verbose) {
			error("Slave did not acknowledge");
		}
		throw I2c::Bus_error();
	}
}


void I2c_imx::Driver::_write(I2c::Bus_address  &bus_address, I2c::Message & m)
{
	/* LSB must be 0 for writing on the bus, address is on the 7 hightest bits */
	_bus_write(bus_address.address << 1);
	m.for_each([&] (unsigned, uint8_t & byte) { _bus_write(byte); });
}


void I2c_imx::Driver::_read(I2c::Bus_address  &bus_address, I2c::Message & m)
{
	using I2c::Mmio;

	/* LSB must be 1 for reading on the bus, address is on the 7 hightest bits */
	_bus_write((uint8_t)(bus_address.address << 1 | 1));

	_mmio.write<Mmio::Control::Tx_rx_select>(0);
	if (m.count() > 1) {
		_mmio.write<Mmio::Control::Tx_ack_enable>(0);
	}
	_mmio.read<Mmio::Data>();

	m.for_each([&] (unsigned idx, uint8_t & byte) {

		do { _wait_for_irq(); }
		while (!_mmio.read<Mmio::Status::Irq>());

		_mmio.write<Mmio::Status::Irq>(0);

		if (idx == m.count() - 1) {
			_mmio.write<Mmio::Control::Tx_rx_select>(0);
			_mmio.write<Mmio::Control::Master_slave_select>(0);
			while (_mmio.read<Mmio::Status::Busy>());
		} else if (idx == m.count() - 2) {
			_mmio.write<Mmio::Control::Tx_ack_enable>(1);
		}

		byte = (uint8_t)_mmio.read<Mmio::Data>();
		_irq.ack();
	});
}


void I2c_imx::Driver::transfer(I2c::Bus_address &a, I2c::Transaction &t)
{
	using I2c::Mmio;

	_bus_start();

	t.for_each([&] (unsigned idx, I2c::Message & m) {
		if (idx > 0) {
			_mmio.write<Mmio::Control::Repeat_start>(1);
			_bus_busy();
		}

		if (m.type == I2c::Message::READ) { _read(a, m);
		} else { _write(a, m); }
	});

	_bus_stop();
}


I2c::Driver_base &_create_driver_instance(Genode::Env &env, I2c::Settings)
{
	static I2c_imx::Driver driver { env, 10, true};

	return driver;
}
