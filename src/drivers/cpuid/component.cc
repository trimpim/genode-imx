
/* Genode includes */
#include <base/component.h>
#include <os/reporter.h>
#include <platform_session/device.h>


namespace Cpuid
{
	using namespace Genode;

	struct Mmio;
	class  Main;
}


struct Cpuid::Mmio : Platform::Device::Mmio
{
	using Ocotp_hw_ocotp_tester0_register = Mmio::Register<0x410, 32>;
	struct Ocotp_hw_ocotp_tester0: Ocotp_hw_ocotp_tester0_register { };

	using Ocotp_hw_ocotp_tester1_register = Mmio::Register<0x420, 32>;
	struct Ocotp_hw_ocotp_tester1 : Ocotp_hw_ocotp_tester1_register { };

	explicit Mmio(Platform::Device &device): Platform::Device::Mmio { device } { }
};


class Cpuid::Main
{
	private:

		Env                  &_env;
		Platform::Connection  _platform { _env };
		Platform::Device      _device   { _platform };
		Mmio                  _mmio     { _device };
		Expanding_reporter    _reporter { _env, "cpuid", "cpuid" };

	public:

		Main(Env &env) :
			_env { env }
		{
			uint64_t cpuid  { static_cast<uint64_t>(_mmio.read<Mmio::Ocotp_hw_ocotp_tester1>()) |
			                  static_cast<uint64_t>(_mmio.read<Mmio::Ocotp_hw_ocotp_tester0>()) << 32 };

			_reporter.generate([&cpuid] (Xml_generator &xml) {
				String<64> uid_str { Hex { cpuid, Hex::OMIT_PREFIX, Hex::PAD } };
				xml.attribute("value", uid_str.string());
			});
		}
};


void Component::construct(Genode::Env &env)
{
	static Cpuid::Main main { env };
}
