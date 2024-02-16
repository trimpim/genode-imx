/*
 * \brief  Driver that connects a UART to a terminal session
 * \author Sebastian Sumpf
 * \date   2022-02-17
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/attached_io_mem_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <irq_session/connection.h>
#include <platform_session/connection.h>
#include <platform_session/device.h>
#include <terminal_session/connection.h>

/* local includes */
#include "base/log.h"
#include "imx_uart.h"


namespace Uart {

	using namespace Genode;
	class Main;

};


class Uart::Main
{
	private:

		Env &_env;

		Attached_rom_dataspace     _config           { _env, "config" };
		bool                       _carriage_return  { _config.xml().attribute_value("carriage_return", false) };

		Platform::Connection       _platform         { _env };
		Platform::Device           _device           { _platform };
		Platform::Device::Irq      _irq              { _device };

		Imx_uart::Driver_settings  _settings;
		Imx_uart                   _uart             { _device, _settings };
		Signal_handler<Main>       _irq_handler      { _env.ep(), *this, &Main::_handle_irq };

		Terminal::Connection       _terminal         { _env };
		Signal_handler<Main>       _terminal_read    { _env.ep(), *this, &Main::_handle_read };

		/*
		 *  Get the serial mode (RS-232 or RS-485) from the config.
		 */
		Imx_uart::Serial_mode _mode_from_config() const
		{
			String<16> mode_str { _config.xml().attribute_value("mode", String<16> { "RS232" }) };

			if (mode_str == "RS232") {
				return Imx_uart::Serial_mode::rs232;
			} else if (mode_str == "RS485") {
				return Imx_uart::Serial_mode::rs485;
			}

			struct Config_error : Exception { };
			Genode::warning("unknown serail mode selected (possible are RS232/RS485)");
			throw Config_error { };
		}

		/*
		 *  Initialize settings from config values.
		 */
		Imx_uart::Driver_settings const _settings_from_config() const
		{
			Imx_uart::Driver_settings const settings {
				.clock_speed { 25000000 },
				.baud_rate   { _config.xml().attribute_value("baud", 115200U) },
				.mode        { _mode_from_config() },
			};
			return settings;
		}

		/*
		 *  Read from the terminal session.
		 */
		void _handle_read()
		{
			while (_terminal.avail()) {

				char buf[32] { };
				size_t size = _terminal.read(buf, 32);
				for (size_t s = 0; s < size; s++) {
					_uart.put_char(buf[s]);
					if (_carriage_return && buf[s] == '\n')
						_uart.put_char('\r');
				}
			}
		}

		/*
		 *  Handle UART IRQs
		 */
		void _handle_irq()
		{
			while (_uart.char_avail()) {
				char c = _uart.get_char();
				_terminal.write(&c, 1);
			}

			_irq.ack();
		}

	public:

		Main(Genode::Env &env) :
			_env(env),
			_settings { _settings_from_config() }
		{
			_irq.sigh(_irq_handler);
			_terminal.read_avail_sigh(_terminal_read);
			_uart.enable_irq();
			_irq.ack();
		}
};


void Component::construct(Genode::Env &env)
{
	static Uart::Main uart { env };
}
