/*
 * \brief  Reset controller for imx8
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

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/component.h>

/* Local includes */
#include "reset_controller_driver.h"

namespace Reset_controller
{
	struct Main;
}

struct Reset_controller::Main
{
	private:
		Genode::Env &_env;

		Reset_controller::Reset_controller_driver _driver {_env};

		Genode::Signal_handler<Reset_controller::Main> _dispatcher;
		Genode::Attached_rom_dataspace _system_report;

		void state_changed()
		{
			_system_report.update();

			if (!_system_report.valid())
				return;

			try {
				Genode::Xml_node system (_system_report.local_addr<char>(),
				                         _system_report.size());

				typedef Genode::String<32> State;
				State const state = system.attribute_value("state", State());

				if (state == "reset") {
					_driver.reset();
				}
			} catch (...) {
				Genode::error("failed to parse system report");
			}
		}

	public:
		Main(Genode::Env &env)
		:
			_env{env},
			_dispatcher(env.ep(), *this, &Reset_controller::Main::state_changed),
			_system_report(_env, "system")
		{
			_system_report.sigh(_dispatcher);
		}
};

void Component::construct(Genode::Env &env) { static Reset_controller::Main main(env); }
