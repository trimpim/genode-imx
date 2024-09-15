/*
 * \brief  Central clock module for i.MX8MM based on i.MX8MQ
 * \author Stefan Kalkowski
 * \author Jean-Adrien Domage
 * \date   2022-11-22
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 * Copyright (C) 2022 gapfruit ag
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#pragma once

#include <os/attached_mmio.h>
#include <clock.h>

namespace Driver {
	using namespace Genode;

	struct Ccm;
};

struct Driver::Ccm
{
	class Frac_pll : public Driver::Clock, Mmio<0x8>
	{
		struct Config_reg_0 : Register<0x0, 32>
		{
			struct Output_div_value : Bitfield<0, 5> {};
			struct Refclk_div_value : Bitfield<5, 6> {};
			struct Newdiv_ack       : Bitfield<11,1> {};
			struct Newdiv_val       : Bitfield<12,1> {};
			struct Bypass           : Bitfield<14,1> {};

			struct Ref_sel          : Bitfield<16,2>
			{
				enum Ref_clk {
					REF_CLK_24M = 0,
					CLK_P_N,
				};
			};

			struct Power_down       : Bitfield<19,1> {};
			struct Out_enable       : Bitfield<21,1> {};
			struct Lock_status      : Bitfield<31,1> {};
		};

		struct Config_reg_1 : Register<0x4, 32>
		{
			struct Int_div_ctl      : Bitfield<0,  7> {};
			struct Frac_div_ctl     : Bitfield<7, 24> {};
		};

		Clocks & _clocks;

		template <typename FN>
		void _parent(FN const & fn) const
		{
			Name pname;

			switch (read<Config_reg_0::Ref_sel>()) {
				case Config_reg_0::Ref_sel::REF_CLK_24M: pname = "24m_ref_clk"; break;
				case Config_reg_0::Ref_sel::CLK_P_N:     pname = "no_clk";      break;
			};

			_clocks.apply(pname, [&] (Clock & clock) { fn(clock); });
		};

		public:

			Frac_pll(Clocks               & clocks,
			         Name                   name,
			         Byte_range_ptr const & range);

			void parent(Name name) override;
			void rate(Rate rate)   override;
			Rate rate()      const override;
			void _enable()         override;
			void _disable()        override;
	};


	class Sccg_pll : public Driver::Clock, Mmio<0xc>
	{
		struct Config_reg_0: Register<0x0, 32>
		{
			struct Ref_sel : Bitfield<0,2> {
				enum Ref_clk {
					REF_CLK_24M = 0,
					CLK_P_N,
				};
			};
			struct Bypass2     : Bitfield<4, 1> {};
			struct Bypass1     : Bitfield<5, 1> {};
			struct Power_down  : Bitfield<7, 1> {};
			struct Out_enable  : Bitfield<25,1> {};
			struct Lock_status : Bitfield<31,1> {};
		};

		struct Config_reg_1 : Register<0x4, 32>
		{
			struct Sse : Bitfield<0,1> {};
		};

		struct Config_reg_2 : Register<0x8, 32>
		{
			struct Output_div_val : Bitfield<1, 6> {};
			struct Feedback_divf2 : Bitfield<7, 6> {};
			struct Feedback_divf1 : Bitfield<13,6> {};
			struct Ref_divr2      : Bitfield<19,6> {};
			struct Ref_divr1      : Bitfield<25,3> {};
		};

		Clocks & _clocks;

		template <typename FN>
		void _parent(FN const & fn) const
		{
			Name pname;

			switch (read<Config_reg_0::Ref_sel>()) {
				case Config_reg_0::Ref_sel::REF_CLK_24M: pname = "24m_ref_clk"; break;
				case Config_reg_0::Ref_sel::CLK_P_N:     pname = "no_clk";      break;
			};

			_clocks.apply(pname, [&] (Clock & clock) { fn(clock); });
		};

		public:

			Sccg_pll(Clocks               & clocks,
			         Name                   name,
			         Byte_range_ptr const & range)
			: Clock(clocks, name), Mmio(range), _clocks(clocks) {}

			void parent(Name name) override;
			void rate(Rate rate)   override;
			Rate rate()      const override;
			void _enable()         override;
			void _disable()        override;
	};


	struct Pllout_monitor : Mmio<0xc>
	{
		struct Config : Register<0x0, 32>
		{
			struct Ref_sel : Bitfield<0,4> {
				enum Ref_clk {
					REF_CLK_24M, CLK_P_N,
					OSC_32K, AUDIO_PLL1, AUDIO_PLL2, GPU_PLL, VPU_PLL,
					VIDEO_PLL1, ARM_PLL, SYS_PLL1, SYS_PLL2, SYS_PLL3,
					VIDEO_PLL2, DRAM_PLL
				};
			};

			struct Enable : Bitfield<4,1> {};
		};

		struct Sccg_divider : Register<0x8, 32>
		{
			struct System_pll_1 : Bitfield<0, 3> {};
		};

		Pllout_monitor(Byte_range_ptr const & range) : Mmio(range) {};
	};


	class Root_clock : public Clock, Mmio<0x4>
	{
		struct Target_reg : Register<0x0, 32>
		{
			struct Post_div : Bitfield<0,6>  {};
			struct Pre_div  : Bitfield<16,3> {};
			struct Ref_sel  : Bitfield<24,3> {};
			struct Enable   : Bitfield<28,1> {};
		};

		struct Clock_ref {
			Clock & ref;
			Clock_ref(Clock & c) : ref(c) {}
		};

		enum { REF_CLK_MAX = 8 };

		Clocks  & _clocks;
		Clock_ref _ref_clks[REF_CLK_MAX];

		Clock & _parent() const;

		public:

			Root_clock(Clocks               & clocks,
			           Name                   name,
			           Byte_range_ptr const & range,
			           Clock                & ref_clk0,
			           Clock                & ref_clk1,
			           Clock                & ref_clk2,
			           Clock                & ref_clk3,
			           Clock                & ref_clk4,
			           Clock                & ref_clk5,
			           Clock                & ref_clk6,
			           Clock                & ref_clk7)
			: Clock(clocks, name), Mmio(range), _clocks(clocks),
			  _ref_clks { ref_clk0, ref_clk1, ref_clk2, ref_clk3,
			              ref_clk4, ref_clk5, ref_clk6, ref_clk7 }{}

			void parent(Name name) override;
			void rate(Rate rate)   override;
			Rate rate()      const override;
			void _enable()         override;
			void _disable()        override;
	};


	class Root_clock_divider : public Clock, Mmio<0x4>
	{
		struct Target_reg : Register<0x0, 32>
		{
			struct Post_div : Bitfield<0,6>  {};
		};


		Clock & _parent;

		public:

			Root_clock_divider(Clocks               & clocks,
			                   Name                   name,
			                   Byte_range_ptr const & range,
			                   Clock                & parent)
			: Clock(clocks, name), Mmio(range),
			  _parent(parent) {}

			void rate(Rate rate) override;
			Rate rate()    const override;
	};


	class Gate : public Clock, Mmio<0x4>
	{
		struct Ccgr : Register<0x0, 32> { };

		Clock & _parent;

		public:

			Gate(Clocks               & clocks,
			     Name                   name,
			     Byte_range_ptr const & range,
			     Clock                & parent)
			: Clock(clocks, name), Mmio(range),
			  _parent(parent) {}

			Rate rate() const override {
				return _parent.rate(); }

			void rate(Rate rate) override {
				_parent.rate(rate); }

			void _enable()       override;
			void _disable()      override;
	};


	enum {
		CCM_MMIO_BASE        = 0x30380000,
		CCM_MMIO_SIZE        = 0x10000,
		CCM_ANALOG_MMIO_BASE = 0x30360000,
		CCM_ANALOG_MMIO_SIZE = 0x10000,
	};

	Ccm(Genode::Env & env, Clocks & clocks);

	Genode::Env    & env;
	Clocks         & clocks;
	Attached_mmio<0> ccm_regs        { env, {(char *)CCM_MMIO_BASE, CCM_MMIO_SIZE} };
	Attached_mmio<0> ccm_analog_regs { env, {(char *)CCM_ANALOG_MMIO_BASE, CCM_ANALOG_MMIO_SIZE} };

	Byte_range_ptr frac_pll_range(unsigned pll) {
		return ccm_analog_regs.range_at(pll*0x8); }

	Byte_range_ptr sccg_pll_range(unsigned pll) {
		return ccm_analog_regs.range_at(0x30 + pll*0xc); }

	Byte_range_ptr gate_range(unsigned nr) {
		return ccm_regs.range_at(0x4000 + nr*0x10); }

	Byte_range_ptr root_range(unsigned nr) {
		return ccm_regs.range_at(0x8000 + nr*0x80); }

	Fixed_clock no_clk           { clocks, "no_clk",           {                 0 }};
	Fixed_clock k32_ref_clk      { clocks, "32k_ref_clk",      {         32 * 1000 }};
	Fixed_clock m24_ref_clk      { clocks, "24m_ref_clk",      {  24 * 1000 * 1000 }};
	Fixed_clock ext_clk_1        { clocks, "ext_clk_1",        { 133 * 1000 * 1000 }};
	Fixed_clock ext_clk_2        { clocks, "ext_clk_2",        { 133 * 1000 * 1000 }};
	Fixed_clock ext_clk_3        { clocks, "ext_clk_3",        { 133 * 1000 * 1000 }};
	Fixed_clock ext_clk_4        { clocks, "ext_clk_4",        { 133 * 1000 * 1000 }};

	Frac_pll audio_pll1_clk { clocks, "audio_pll1_clk", frac_pll_range(0) };
	Frac_pll audio_pll2_clk { clocks, "audio_pll2_clk", frac_pll_range(1) };
	Frac_pll video_pll1_clk { clocks, "video_pll1_clk", frac_pll_range(2) };
	Frac_pll gpu_pll_clk    { clocks, "gpu_pll_clk",    frac_pll_range(3) };
	Frac_pll vpu_pll_clk    { clocks, "vpu_pll_clk",    frac_pll_range(4) };
	Frac_pll arm_pll_clk    { clocks, "arm_pll_clk",    frac_pll_range(5) };

	Fixed_clock system_pll1_clk { clocks, "system_pll1_clk", { 800000000  } };
	Fixed_clock system_pll2_clk { clocks, "system_pll2_clk", { 1000000000 } };
	Fixed_clock system_pll3_clk { clocks, "system_pll3_clk", { 1000000000 } };

	Fixed_divider system_pll1_div20 { clocks, "system_pll1_div20", system_pll1_clk, {20} };
	Fixed_divider system_pll1_div10 { clocks, "system_pll1_div10", system_pll1_clk, {10} };
	Fixed_divider system_pll1_div8  { clocks, "system_pll1_div8",  system_pll1_clk, { 8} };
	Fixed_divider system_pll1_div6  { clocks, "system_pll1_div6",  system_pll1_clk, { 6} };
	Fixed_divider system_pll1_div5  { clocks, "system_pll1_div5",  system_pll1_clk, { 5} };
	Fixed_divider system_pll1_div4  { clocks, "system_pll1_div4",  system_pll1_clk, { 4} };
	Fixed_divider system_pll1_div3  { clocks, "system_pll1_div3",  system_pll1_clk, { 3} };
	Fixed_divider system_pll1_div2  { clocks, "system_pll1_div2",  system_pll1_clk, { 2} };
	Fixed_divider system_pll2_div20 { clocks, "system_pll2_div20", system_pll2_clk, {20} };
	Fixed_divider system_pll2_div10 { clocks, "system_pll2_div10", system_pll2_clk, {10} };
	Fixed_divider system_pll2_div8  { clocks, "system_pll2_div8",  system_pll2_clk, { 8} };
	Fixed_divider system_pll2_div6  { clocks, "system_pll2_div6",  system_pll2_clk, { 6} };
	Fixed_divider system_pll2_div5  { clocks, "system_pll2_div5",  system_pll2_clk, { 5} };
	Fixed_divider system_pll2_div4  { clocks, "system_pll2_div4",  system_pll2_clk, { 4} };
	Fixed_divider system_pll2_div3  { clocks, "system_pll2_div3",  system_pll2_clk, { 3} };
	Fixed_divider system_pll2_div2  { clocks, "system_pll2_div2",  system_pll2_clk, { 2} };

	Root_clock arm_a53_clk_root              { clocks, "arm_a53_clk_root",
	                                           root_range(0), m24_ref_clk,
	                                           arm_pll_clk, system_pll2_div2,
	                                           system_pll2_clk, system_pll1_clk,
	                                           system_pll1_div2, audio_pll1_clk, system_pll3_clk };
	Root_clock arm_m4_clk_root               { clocks, "arm_m4_clk_root",
	                                           root_range(1), m24_ref_clk,
	                                           system_pll2_div5, system_pll2_div4,
	                                           system_pll1_div3, system_pll1_clk,
	                                           audio_pll1_clk, video_pll1_clk, system_pll3_clk };
	Root_clock vpu_a53_clk_root              { clocks, "vpu_a53_clk_root",
	                                           root_range(2), m24_ref_clk,
	                                           arm_pll_clk, system_pll2_div2,
	                                           system_pll2_clk, system_pll1_clk,
	                                           system_pll1_div2, audio_pll1_clk, vpu_pll_clk };
	Root_clock gpu_core_clk_root             { clocks, "gpu_core_clk_root",
	                                           root_range(3), m24_ref_clk,
	                                           gpu_pll_clk, system_pll1_clk,
	                                           system_pll3_clk, system_pll2_clk,
	                                           audio_pll1_clk, video_pll1_clk, audio_pll2_clk };
	Root_clock gpu_shader_clk_root           { clocks, "gpu_shader_clk",
	                                           root_range(4), m24_ref_clk,
	                                           gpu_pll_clk, system_pll1_clk,
	                                           system_pll3_clk, system_pll2_clk,
	                                           audio_pll1_clk, video_pll1_clk, audio_pll2_clk };
	Root_clock main_axi_clk_root             { clocks, "main_axi_clk_root",
	                                           root_range(16), m24_ref_clk,
	                                           system_pll2_div3, system_pll1_clk,
	                                           system_pll2_div4, system_pll2_clk,
	                                           audio_pll1_clk, video_pll1_clk, system_pll1_div8 };
	Root_clock enet_axi_clk_root             { clocks, "enet_axi_clk_root",
	                                           root_range(17), m24_ref_clk,
	                                           system_pll1_div3, system_pll1_clk,
	                                           system_pll2_div4, system_pll2_div5,
	                                           audio_pll1_clk, video_pll1_clk, system_pll3_clk };
	Root_clock nand_usdhc_bus_clk_root       { clocks, "nand_usdhc_bus_clk_root",
	                                           root_range(18), m24_ref_clk,
	                                           system_pll1_div3, system_pll1_clk,
	                                           system_pll2_div5, system_pll1_div6,
	                                           system_pll3_clk, system_pll2_div4, audio_pll1_clk };
	Root_clock vpu_bus_clk_root              { clocks, "vpu_bus_clk_root",
	                                           root_range(19), m24_ref_clk,
	                                           system_pll1_clk, vpu_pll_clk,
	                                           audio_pll2_clk, system_pll3_clk,
	                                           system_pll2_clk, system_pll2_div5, system_pll1_div8 };
	Root_clock display_axi_clk_root          { clocks, "display_axi_clk_root",
	                                           root_range(20), m24_ref_clk,
	                                           system_pll2_div8, system_pll1_clk,
	                                           system_pll3_clk, system_pll1_div20,
	                                           audio_pll2_clk, ext_clk_1, ext_clk_4 };
	Root_clock display_apb_clk_root          { clocks, "display_apb_clk_root",
	                                           root_range(21), m24_ref_clk,
	                                           system_pll2_div8, system_pll1_clk,
	                                           system_pll3_clk, system_pll1_div20,
	                                           audio_pll2_clk, ext_clk_1, ext_clk_3 };
	Root_clock display_rtrm_clk_root         { clocks, "display_rtrm_clk_root",
	                                           root_range(22), m24_ref_clk,
	                                           system_pll1_clk, system_pll2_div5,
	                                           system_pll1_div2, audio_pll1_clk,
	                                           video_pll1_clk, ext_clk_2, ext_clk_3 };
	Root_clock usb_bus_clk_root              { clocks, "usb_bus_clk_root",
	                                           root_range(23), m24_ref_clk,
	                                           system_pll2_div2, system_pll1_clk,
	                                           system_pll2_div10, system_pll2_div5,
	                                           ext_clk_2, ext_clk_4, audio_pll2_clk };
	Root_clock gpu_axi_clk_root              { clocks, "gpu_axi_clk_root",
	                                           root_range(24), m24_ref_clk,
	                                           system_pll1_clk, gpu_pll_clk,
	                                           system_pll3_clk, system_pll2_clk,
	                                           audio_pll1_clk, video_pll1_clk, audio_pll2_clk };
	Root_clock gpu_ahb_clk_root              { clocks, "gpu_ahb_clk_root",
	                                           root_range(25), m24_ref_clk,
	                                           system_pll1_clk, gpu_pll_clk,
	                                           system_pll3_clk, system_pll2_clk,
	                                           audio_pll1_clk, video_pll1_clk, audio_pll2_clk };
	Root_clock noc_clk_root                  { clocks, "noc_clk_root",
	                                           root_range(26), m24_ref_clk,
	                                           system_pll1_clk, system_pll3_clk,
	                                           system_pll2_clk, system_pll2_div2,
	                                           audio_pll1_clk, video_pll1_clk, audio_pll2_clk };
	Root_clock noc_apb_clk_root              { clocks, "noc_apb_clk_root",
	                                           root_range(27), m24_ref_clk,
	                                           system_pll1_div2, system_pll3_clk,
	                                           system_pll2_div3, system_pll2_div5,
	                                           system_pll1_clk, audio_pll1_clk, video_pll1_clk };
	Root_clock ahb_clk_root                  { clocks, "ahb_clk_root",
	                                           root_range(32), m24_ref_clk,
	                                           system_pll1_div6, system_pll1_clk,
	                                           system_pll1_div2, system_pll2_div8,
	                                           system_pll3_clk, audio_pll1_clk, video_pll1_clk };
	Root_clock audio_ahb_clk_root            { clocks, "audio_ahb_clk_root",
	                                           root_range(34), m24_ref_clk,
	                                           system_pll2_div2, system_pll1_clk,
	                                           system_pll2_clk, system_pll2_div6,
	                                           system_pll3_clk, audio_pll1_clk, video_pll1_clk };
	Root_clock mipi_dsi_esc_rx_clk_root      { clocks, "mipi_dsi_esc_rx_clk_root",
	                                           root_range(36), m24_ref_clk,
	                                           system_pll2_div10, system_pll1_div10,
	                                           system_pll1_clk, system_pll2_clk,
	                                           system_pll3_clk, ext_clk_3, audio_pll2_clk };
	Root_clock dram_alt_clk_root             { clocks, "dram_alt_clk_root",
	                                           root_range(64), m24_ref_clk,
	                                           system_pll1_clk, system_pll1_div8,
	                                           system_pll2_div2, system_pll2_div4,
	                                           system_pll1_div2, audio_pll1_clk, system_pll1_div3 };
	Root_clock dram_apb_clk_root             { clocks, "dram_apb_clk_root",
	                                           root_range(65), m24_ref_clk,
	                                           system_pll2_div5, system_pll1_div20,
	                                           system_pll1_div5, system_pll1_clk,
	                                           system_pll3_clk, system_pll2_div4, audio_pll2_clk };
	Root_clock vpu_g1_clk_root               { clocks, "vpu_g1_clk_root",
	                                           root_range(66), m24_ref_clk,
	                                           vpu_pll_clk, system_pll1_clk,
	                                           system_pll2_clk, system_pll1_div8,
	                                           system_pll2_div8, system_pll3_clk, audio_pll1_clk };
	Root_clock vpu_g2_clk_root               { clocks, "vpu_g2_clk_root",
	                                           root_range(67), m24_ref_clk,
	                                           vpu_pll_clk, system_pll1_clk,
	                                           system_pll2_clk, system_pll1_div8,
	                                           system_pll2_div8, system_pll3_clk, audio_pll1_clk };
	Root_clock display_dtrc_clk_root         { clocks, "display_dtrc_clk_root",
	                                           root_range(68), m24_ref_clk,
	                                           no_clk, system_pll1_clk,
	                                           system_pll2_clk, system_pll1_div5,
	                                           video_pll1_clk, system_pll3_clk, audio_pll2_clk };
	Root_clock display_dc8000_clk_root       { clocks, "display_dc8000_clk_root",
	                                           root_range(69), m24_ref_clk,
	                                           no_clk, system_pll1_clk,
	                                           system_pll2_clk, system_pll1_div5,
	                                           video_pll1_clk, system_pll3_clk, audio_pll2_clk };
	Root_clock pcie1_ctrl_clk_root           { clocks, "pcie1_ctrl_clk_root",
	                                           root_range(70), m24_ref_clk,
	                                           system_pll2_div4, system_pll2_div5,
	                                           system_pll1_div3, system_pll1_clk,
	                                           system_pll2_div2, system_pll2_div3, system_pll3_clk };
	Root_clock pcie1_phy_clk_root            { clocks, "pcie1_phy_clk_root",
	                                           root_range(71), m24_ref_clk,
	                                           system_pll2_div10, system_pll2_div2,
	                                           ext_clk_1, ext_clk_2,
	                                           ext_clk_3, ext_clk_4, system_pll1_div2 };
	Root_clock pcie1_aux_clk_root            { clocks, "pcie1_aux_clk_root",
	                                           root_range(72), m24_ref_clk,
	                                           system_pll2_div5, system_pll2_div20,
	                                           system_pll3_clk, system_pll2_div10,
	                                           system_pll1_div10, system_pll1_div5, system_pll1_div4 };
	Root_clock dc_pixel_clk_root             { clocks, "dc_pixel_clk_root",
	                                           root_range(73), m24_ref_clk,
	                                           video_pll1_clk, audio_pll2_clk,
	                                           audio_pll1_clk, system_pll1_clk,
	                                           system_pll2_clk, system_pll3_clk, ext_clk_4 };
	Root_clock lcdif_pixel_clk_root          { clocks, "lcdif_pixel_clk_root",
	                                           root_range(74), m24_ref_clk,
	                                           video_pll1_clk, audio_pll2_clk,
	                                           audio_pll1_clk, system_pll1_clk,
	                                           system_pll2_clk, system_pll3_clk, ext_clk_4 };
	Root_clock sai1_clk_root                 { clocks, "sai1_clk_root",
	                                           root_range(75), m24_ref_clk,
	                                           audio_pll1_clk, audio_pll2_clk,
	                                           video_pll1_clk, system_pll1_div6,
	                                           no_clk, ext_clk_1, ext_clk_2 };
	Root_clock sai2_clk_root                 { clocks, "sai2_clk_root",
	                                           root_range(76), m24_ref_clk,
	                                           audio_pll1_clk, audio_pll2_clk,
	                                           video_pll1_clk, system_pll1_div6,
	                                           no_clk, ext_clk_2, ext_clk_3 };
	Root_clock sai3_clk_root                 { clocks, "sai3_clk_root",
	                                           root_range(77), m24_ref_clk,
	                                           audio_pll1_clk, audio_pll2_clk,
	                                           video_pll1_clk, system_pll1_div6,
	                                           no_clk, ext_clk_3, ext_clk_4 };
	Root_clock sai5_clk_root                 { clocks, "sai5_clk_root",
	                                           root_range(79), m24_ref_clk,
	                                           audio_pll1_clk, audio_pll2_clk,
	                                           video_pll1_clk, system_pll1_div6,
	                                           no_clk, ext_clk_2, ext_clk_3 };
	Root_clock sai6_clk_root                 { clocks, "sai6_clk_root",
	                                           root_range(80), m24_ref_clk,
	                                           audio_pll1_clk, audio_pll2_clk,
	                                           video_pll1_clk, system_pll1_div6,
	                                           no_clk, ext_clk_3, ext_clk_4 };
	Root_clock spdif1_clk_root               { clocks, "spdif1_clk_root",
	                                           root_range(81), m24_ref_clk,
	                                           audio_pll1_clk, audio_pll2_clk,
	                                           video_pll1_clk, system_pll1_div6,
	                                           no_clk, ext_clk_2, ext_clk_3 };
	Root_clock spdif2_clk_root               { clocks, "spdif2_clk_root",
	                                           root_range(82), m24_ref_clk,
	                                           audio_pll1_clk, audio_pll2_clk,
	                                           video_pll1_clk, system_pll1_div6,
	                                           no_clk, ext_clk_3, ext_clk_4 };
	Root_clock enet_ref_clk_root             { clocks, "enet_ref_clk_root",
	                                           root_range(83), m24_ref_clk,
	                                           system_pll2_div8, system_pll2_div20,
	                                           system_pll2_div10, system_pll1_div5,
	                                           audio_pll1_clk, video_pll1_clk, ext_clk_4 };
	Root_clock enet_timer_clk_root           { clocks, "enet_timer_clk_root",
	                                           root_range(84), m24_ref_clk,
	                                           system_pll2_div10, audio_pll1_clk,
	                                           ext_clk_1, ext_clk_2,
	                                           ext_clk_3, ext_clk_4, video_pll1_clk };
	Root_clock enet_phy_ref_clk_root         { clocks, "enet_phy_ref_clk_root",
	                                           root_range(85), m24_ref_clk,
	                                           system_pll2_div20, system_pll2_div8,
	                                           system_pll2_div5, system_pll2_div2,
	                                           audio_pll1_clk, video_pll1_clk, audio_pll2_clk };
	Root_clock nand_clk_root                 { clocks, "nand_clk_root",
	                                           root_range(86), m24_ref_clk,
	                                           system_pll2_div2, audio_pll1_clk,
	                                           system_pll1_div2, audio_pll2_clk,
	                                           system_pll3_clk, system_pll2_div4, video_pll1_clk };
	Root_clock qspi_clk_root                 { clocks, "qspi_clk_root",
	                                           root_range(87), m24_ref_clk,
	                                           system_pll1_div2, system_pll1_clk,
	                                           system_pll2_div2, audio_pll2_clk,
	                                           system_pll1_div3, system_pll3_clk, system_pll1_div8 };
	Root_clock usdhc1_clk_root               { clocks, "usdhc1_clk_root",
	                                           root_range(88), m24_ref_clk,
	                                           system_pll1_div2, system_pll1_clk,
	                                           system_pll2_div2, system_pll3_clk,
	                                           system_pll1_div3, audio_pll2_clk, system_pll1_div8 };
	Root_clock usdhc2_clk_root               { clocks, "usdhc2_clk_root",
	                                           root_range(89), m24_ref_clk,
	                                           system_pll1_div2, system_pll1_clk,
	                                           system_pll2_div2, system_pll3_clk,
	                                           system_pll1_div3, audio_pll2_clk, system_pll1_div8 };
	Root_clock i2c1_clk_root                 { clocks, "i2c1_clk_root",
	                                           root_range(90), m24_ref_clk,
	                                           system_pll1_div5, system_pll2_div20,
	                                           system_pll3_clk, audio_pll1_clk,
	                                           video_pll1_clk, audio_pll2_clk, system_pll1_div6 };
	Root_clock i2c2_clk_root                 { clocks, "i2c2_clk_root",
	                                           root_range(91), m24_ref_clk,
	                                           system_pll1_div5, system_pll2_div20,
	                                           system_pll3_clk, audio_pll1_clk,
	                                           video_pll1_clk, audio_pll2_clk, system_pll1_div6 };
	Root_clock i2c3_clk_root                 { clocks, "i2c3_clk_root",
	                                           root_range(92), m24_ref_clk,
	                                           system_pll1_div5, system_pll2_div20,
	                                           system_pll3_clk, audio_pll1_clk,
	                                           video_pll1_clk, audio_pll2_clk, system_pll1_div6 };
	Root_clock i2c4_clk_root                 { clocks, "i2c4_clk_root",
	                                           root_range(93), m24_ref_clk,
	                                           system_pll1_div5, system_pll2_div20,
	                                           system_pll3_clk, audio_pll1_clk,
	                                           video_pll1_clk, audio_pll2_clk, system_pll1_div6 };
	Root_clock uart1_clk_root                { clocks, "uart1_clk_root",
	                                           root_range(94), m24_ref_clk,
	                                           system_pll1_div10, system_pll2_div5,
	                                           system_pll2_div10, system_pll3_clk,
	                                           ext_clk_2, ext_clk_4, audio_pll2_clk };
	Root_clock uart2_clk_root                { clocks, "uart2_clk_root",
	                                           root_range(95), m24_ref_clk,
	                                           system_pll1_div10, system_pll2_div5,
	                                           system_pll2_div10, system_pll3_clk,
	                                           ext_clk_2, ext_clk_3, audio_pll2_clk };
	Root_clock uart3_clk_root                { clocks, "uart3_clk_root",
	                                           root_range(96), m24_ref_clk,
	                                           system_pll1_div10, system_pll2_div5,
	                                           system_pll2_div10, system_pll3_clk,
	                                           ext_clk_2, ext_clk_4, audio_pll2_clk };
	Root_clock uart4_clk_root                { clocks, "uart4_clk_root",
	                                           root_range(97), m24_ref_clk,
	                                           system_pll1_div10, system_pll2_div5,
	                                           system_pll2_div10, system_pll3_clk,
	                                           ext_clk_2, ext_clk_3, audio_pll2_clk };
	Root_clock usb_core_ref_clk_root         { clocks, "usb_core_ref_clk_root",
	                                           root_range(98), m24_ref_clk,
	                                           system_pll1_div8, system_pll1_div20,
	                                           system_pll2_div10, system_pll2_div5,
	                                           ext_clk_2, ext_clk_3, audio_pll2_clk };
	Root_clock usb_phy_ref_clk_root          { clocks, "usb_phy_ref_clk_root",
	                                           root_range(99), m24_ref_clk,
	                                           system_pll1_div8, system_pll1_div20,
	                                           system_pll2_div10, system_pll2_div5,
	                                           ext_clk_2, ext_clk_3, audio_pll2_clk };
	Root_clock gic_clk_root                  { clocks, "gic_clk_root",
	                                           root_range(100), m24_ref_clk,
	                                           system_pll2_div5, system_pll1_div20,
	                                           system_pll2_div10, system_pll1_clk,
	                                           ext_clk_2, ext_clk_4, audio_pll2_clk };
	Root_clock ecspi1_clk_root               { clocks, "ecspi1_clk_root",
	                                           root_range(101), m24_ref_clk,
	                                           system_pll2_div5, system_pll1_div20,
	                                           system_pll1_div5, system_pll1_clk,
	                                           system_pll3_clk, system_pll2_div4, audio_pll2_clk };
	Root_clock ecspi2_clk_root               { clocks, "ecspi2_clk_root",
	                                           root_range(102), m24_ref_clk,
	                                           system_pll2_div5, system_pll1_div20,
	                                           system_pll1_div5, system_pll1_clk,
	                                           system_pll3_clk, system_pll2_div4, audio_pll2_clk };
	Root_clock pwm1_clk_root                 { clocks, "pwm1_clk_root",
	                                           root_range(103), m24_ref_clk,
	                                           system_pll2_div10, system_pll1_div5,
	                                           system_pll1_div20, system_pll3_clk,
	                                           ext_clk_1, system_pll1_div10, video_pll1_clk };
	Root_clock pwm2_clk_root                 { clocks, "pwm2_clk_root",
	                                           root_range(104), m24_ref_clk,
	                                           system_pll2_div10, system_pll1_div5,
	                                           system_pll1_div20, system_pll3_clk,
	                                           ext_clk_1, system_pll1_div10, video_pll1_clk };
	Root_clock pwm3_clk_root                 { clocks, "pwm3_clk_root",
	                                           root_range(105), m24_ref_clk,
	                                           system_pll2_div10, system_pll1_div5,
	                                           system_pll1_div20, system_pll3_clk,
	                                           ext_clk_2, system_pll1_div10, video_pll1_clk };
	Root_clock pwm4_clk_root                 { clocks, "pwm4_clk_root",
	                                           root_range(106), m24_ref_clk,
	                                           system_pll2_div10, system_pll1_div5,
	                                           system_pll1_div20, system_pll3_clk,
	                                           ext_clk_2, system_pll1_div10, video_pll1_clk };
	Root_clock gpt1_clk_root                 { clocks, "gpt1_clk_root",
	                                           root_range(107), m24_ref_clk,
	                                           system_pll2_div10, system_pll1_div2,
	                                           system_pll1_div20, video_pll1_clk,
	                                           system_pll1_div10, audio_pll1_clk, ext_clk_1 };
	Root_clock gpt2_clk_root                 { clocks, "gpt2_clk_root",
	                                           root_range(108), m24_ref_clk,
	                                           system_pll2_div10, system_pll1_div2,
	                                           system_pll1_div20, video_pll1_clk,
	                                           system_pll1_div10, audio_pll1_clk, ext_clk_2 };
	Root_clock gpt3_clk_root                 { clocks, "gpt3_clk_root",
	                                           root_range(109), m24_ref_clk,
	                                           system_pll2_div10, system_pll1_div2,
	                                           system_pll1_div20, video_pll1_clk,
	                                           system_pll1_div10, audio_pll1_clk, ext_clk_3 };
	Root_clock gpt4_clk_root                 { clocks, "gpt4_clk_root",
	                                           root_range(110), m24_ref_clk,
	                                           system_pll2_div10, system_pll1_div2,
	                                           system_pll1_div20, video_pll1_clk,
	                                           system_pll1_div10, audio_pll1_clk, ext_clk_1 };
	Root_clock gpt5_clk_root                 { clocks, "gpt5_clk_root",
	                                           root_range(111), m24_ref_clk,
	                                           system_pll2_div10, system_pll1_div2,
	                                           system_pll1_div20, video_pll1_clk,
	                                           system_pll1_div10, audio_pll1_clk, ext_clk_2 };
	Root_clock gpt6_clk_root                 { clocks, "gpt6_clk_root",
	                                           root_range(112), m24_ref_clk,
	                                           system_pll2_div10, system_pll1_div2,
	                                           system_pll1_div20, video_pll1_clk,
	                                           system_pll1_div10, audio_pll1_clk, ext_clk_3 };
	Root_clock trace_clk_root                { clocks, "trace_clk_root",
	                                           root_range(113), m24_ref_clk,
	                                           system_pll1_div6, system_pll1_div5,
	                                           vpu_pll_clk, system_pll2_div8,
	                                           system_pll3_clk, ext_clk_1, ext_clk_3 };
	Root_clock wdog_clk_root                 { clocks, "wdog_clk_root",
	                                           root_range(114), m24_ref_clk,
	                                           system_pll1_div6, system_pll1_div5,
	                                           vpu_pll_clk, system_pll2_div8,
	                                           system_pll3_clk, system_pll1_div10, system_pll2_div6 };
	Root_clock wrclk_clk_root                { clocks, "wrclk_clk_root",
	                                           root_range(115), m24_ref_clk,
	                                           system_pll1_div20, vpu_pll_clk,
	                                           system_pll3_clk, system_pll2_div5,
	                                           system_pll1_div3, system_pll2_div2, system_pll1_div8 };
	Root_clock ipp_do_clko1clk_root          { clocks, "ipp_do_clko1_clk_root",
	                                           root_range(116), m24_ref_clk,
	                                           system_pll1_div20, vpu_pll_clk,
	                                           system_pll1_div4, audio_pll2_clk,
	                                           system_pll2_div2, vpu_pll_clk, system_pll1_div10 };
	Root_clock ipp_do_clko2_clk_root         { clocks, "ipp_do_clko2_clk_root",
	                                           root_range(117), m24_ref_clk,
	                                           system_pll2_div5, system_pll1_div2,
	                                           system_pll2_div6, system_pll3_clk,
	                                           audio_pll1_clk, video_pll1_clk, k32_ref_clk };
	Root_clock mipi_dsi_core_clk_root        { clocks, "mipi_dsi_core_clk_root",
	                                           root_range(118), m24_ref_clk,
	                                           system_pll1_div3, system_pll2_div4,
	                                           system_pll1_clk, system_pll2_clk,
	                                           system_pll3_clk, audio_pll2_clk, video_pll1_clk };
	Root_clock mipi_dsi_phy_ref_clk_root     { clocks, "mipi_dsi_phy_ref_clk_root",
	                                           root_range(119), m24_ref_clk,
	                                           system_pll2_div8, system_pll2_div10,
	                                           system_pll1_clk, system_pll2_clk,
	                                           ext_clk_2, audio_pll2_clk, video_pll1_clk };
	Root_clock mipi_dsi_dbi_clk_root         { clocks, "mipi_dsi_dbi_clk_root",
	                                           root_range(120), m24_ref_clk,
	                                           system_pll1_div3, system_pll2_div10,
	                                           system_pll1_clk, system_pll2_clk,
	                                           system_pll3_clk, audio_pll2_clk, video_pll1_clk };
	Root_clock usdhc3_clk_root               { clocks, "usdhc3_clk_root",
	                                           root_range(121), m24_ref_clk,
	                                           system_pll1_div2, system_pll1_clk,
	                                           system_pll2_div2, system_pll3_clk,
	                                           system_pll1_div3, audio_pll2_clk, system_pll1_div8 };
	Root_clock mipi_csi1_core_clk_root       { clocks, "mipi_csi1_core_clk_root",
	                                           root_range(122), m24_ref_clk,
	                                           system_pll1_div3, system_pll2_div4,
	                                           system_pll1_clk, system_pll2_clk,
	                                           system_pll3_clk, audio_pll2_clk, video_pll1_clk };
	Root_clock mipi_csi1_phy_ref_clk_root    { clocks, "mipi_csi1_phy_ref_clk_root",
	                                           root_range(123), m24_ref_clk,
	                                           system_pll2_div3, system_pll2_div10,
	                                           system_pll1_clk, system_pll2_clk,
	                                           ext_clk_2, audio_pll2_clk, video_pll1_clk };
	Root_clock mipi_csi1_esc_clk_root        { clocks, "mipi_csi1_esc_clk_root",
	                                           root_range(124), m24_ref_clk,
	                                           system_pll2_div10, system_pll1_div10,
	                                           system_pll1_clk, system_pll2_clk,
	                                           system_pll3_clk, ext_clk_3, audio_pll2_clk };
	Root_clock mipi_csi2_core_clk_root       { clocks, "mipi_csi2_core_clk_root",
	                                           root_range(125), m24_ref_clk,
	                                           system_pll1_div3, system_pll2_div4,
	                                           system_pll1_clk, system_pll2_clk,
	                                           system_pll3_clk, audio_pll2_clk, video_pll1_clk };
	Root_clock mipi_csi2_phy_ref_clk_root    { clocks, "mipi_csi2_phy_ref_clk_root",
	                                           root_range(126), m24_ref_clk,
	                                           system_pll2_div3, system_pll2_div10,
	                                           system_pll1_clk, system_pll2_clk,
	                                           ext_clk_2, audio_pll2_clk, video_pll1_clk };
	Root_clock mipi_csi2_esc_clk_root        { clocks, "mipi_csi2_esc_clk_root",
	                                           root_range(127), m24_ref_clk,
	                                           system_pll2_div10, system_pll1_div10,
	                                           system_pll1_clk, system_pll2_clk,
	                                           system_pll3_clk, ext_clk_3, audio_pll2_clk };
	Root_clock pcie2_ctrl_clk_root           { clocks, "pcie2_ctrl_clk_root",
	                                           root_range(128), m24_ref_clk,
	                                           system_pll2_div4, system_pll2_div5,
	                                           system_pll1_div3, system_pll1_clk,
	                                           system_pll2_div2, system_pll2_div3, system_pll3_clk };
	Root_clock pcie2_phy_clk_root            { clocks, "pcie2_phy_clk_root",
	                                           root_range(129), m24_ref_clk,
	                                           system_pll2_div10, system_pll2_div2,
	                                           ext_clk_1, ext_clk_2,
	                                           ext_clk_3, ext_clk_4, system_pll1_div2 };
	Root_clock pcie2_aux_clk_root            { clocks, "pcie2_aux_clk_root",
	                                           root_range(130), m24_ref_clk,
	                                           system_pll2_div5, system_pll2_div20,
	                                           system_pll3_clk, system_pll2_div10,
	                                           system_pll1_div10, system_pll1_div5, system_pll1_div4 };
	Root_clock ecspi3_clk_root               { clocks, "ecspi3_clk_root",
	                                           root_range(131), m24_ref_clk,
	                                           system_pll2_div5, system_pll1_div20,
	                                           system_pll1_div5, system_pll1_clk,
	                                           system_pll3_clk, system_pll2_div4, audio_pll2_clk };
	Root_clock old_mipi_dsi_esc_rx_clk_root  { clocks, "old_mipi_dsi_esc_rx_clk_root",
	                                           root_range(132), m24_ref_clk,
	                                           system_pll2_div10, system_pll1_div10,
	                                           system_pll1_clk, system_pll2_clk,
	                                           system_pll3_clk, ext_clk_3, audio_pll2_clk };
	Root_clock display_hdmi_clk_root         { clocks, "display_hdmi_clk_root",
	                                           root_range(133), m24_ref_clk,
	                                           system_pll1_div4, system_pll2_div5,
	                                           vpu_pll_clk, system_pll1_clk,
	                                           system_pll2_clk, system_pll3_clk, ext_clk_4 };

	Root_clock_divider ipg_clk_root          { clocks, "ipg_clk_root",
	                                           root_range(33), ahb_clk_root };
	Root_clock_divider ipg_audio_clk_root    { clocks, "ipg_audio_clk_root",
	                                           root_range(35), audio_ahb_clk_root };
	Root_clock_divider mipi_dsi_esc_clk_root { clocks, "mipi_dsi_esc_clk_root",
	                                           root_range(37), mipi_dsi_esc_rx_clk_root };

	Gate ecspi1_gate     { clocks, "ecspi1_gate",     gate_range(7),   ecspi1_clk_root         };
	Gate ecspi2_gate     { clocks, "ecspi2_gate",     gate_range(8),   ecspi2_clk_root         };
	Gate ecspi3_gate     { clocks, "ecspi3_gate",     gate_range(9),   ecspi3_clk_root         };
	Gate enet1_gate      { clocks, "enet1_gate",      gate_range(10),  enet_ref_clk_root       };
	Gate gpt1_gate       { clocks, "gpt1_gate",       gate_range(16),  gpt1_clk_root           };
	Gate i2c1_gate       { clocks, "i2c1_gate",       gate_range(23),  i2c1_clk_root           };
	Gate i2c2_gate       { clocks, "i2c2_gate",       gate_range(24),  i2c2_clk_root           };
	Gate i2c3_gate       { clocks, "i2c3_gate",       gate_range(25),  i2c3_clk_root           };
	Gate i2c4_gate       { clocks, "i2c4_gate",       gate_range(26),  i2c4_clk_root           };
	Gate mu_gate         { clocks, "mu_gate",         gate_range(33),  ipg_clk_root            };
	Gate ocotp_gate      { clocks, "ocotp_gate",      gate_range(34),  ipg_clk_root            };
	Gate pcie_gate       { clocks, "pcie_gate",       gate_range(37),  pcie1_ctrl_clk_root     };
	Gate pwm1_gate       { clocks, "pwm1_gate",       gate_range(40),  pwm1_clk_root           };
	Gate pwm2_gate       { clocks, "pwm2_gate",       gate_range(41),  pwm2_clk_root           };
	Gate pwm3_gate       { clocks, "pwm3_gate",       gate_range(42),  pwm3_clk_root           };
	Gate pwm4_gate       { clocks, "pwm4_gate",       gate_range(43),  pwm4_clk_root           };
	Gate qspi_gate       { clocks, "qspi_gate",       gate_range(47),  qspi_clk_root           };
	Gate nand_gate       { clocks, "nand_gate",       gate_range(48),  nand_clk_root           };
	Gate sai1_gate       { clocks, "sai1_gate",       gate_range(51),  sai1_clk_root           };
	Gate sai2_gate       { clocks, "sai2_gate",       gate_range(52),  sai2_clk_root           };
	Gate sai3_gate       { clocks, "sai3_gate",       gate_range(53),  sai3_clk_root           };
	Gate sai5_gate       { clocks, "sai5_gate",       gate_range(55),  sai5_clk_root           };
	Gate sai6_gate       { clocks, "sai6_gate",       gate_range(56),  sai6_clk_root           };
	Gate sdma1_gate      { clocks, "sdma1_gate",      gate_range(58),  ipg_clk_root            };
	Gate sdma2_gate      { clocks, "sdma2_gate",      gate_range(59),  ipg_audio_clk_root      };
	Gate uart1_gate      { clocks, "uart1_gate",      gate_range(73),  uart1_clk_root          };
	Gate uart2_gate      { clocks, "uart2_gate",      gate_range(74),  uart2_clk_root          };
	Gate uart3_gate      { clocks, "uart3_gate",      gate_range(75),  uart3_clk_root          };
	Gate uart4_gate      { clocks, "uart4_gate",      gate_range(76),  uart4_clk_root          };
	Gate usb_ctrl1_gate  { clocks, "usb_ctrl1_gate",  gate_range(77),  usb_core_ref_clk_root   };
	Gate usdhc1_gate     { clocks, "usdhc1_gate",     gate_range(81),  usdhc1_clk_root         };
	Gate usdhc2_gate     { clocks, "usdhc2_gate",     gate_range(82),  usdhc2_clk_root         };
	Gate wdog1_gate      { clocks, "wdog1_gate",      gate_range(83),  wdog_clk_root           };
	Gate wdog2_gate      { clocks, "wdog2_gate",      gate_range(84),  wdog_clk_root           };
	Gate wdog3_gate      { clocks, "wdog3_gate",      gate_range(85),  wdog_clk_root           };
	Gate va53_gate       { clocks, "va53_gate",       gate_range(86),  vpu_g1_clk_root         };
	Gate vp9_gate        { clocks, "vp9_gate",        gate_range(90),  vpu_g2_clk_root         };
	Gate usdhc3_gate     { clocks, "usdhc3_gate",     gate_range(94),  usdhc3_clk_root         };
	Gate tempsensor_gate { clocks, "tempsensor_gate", gate_range(98),  ipg_clk_root            };
	Gate vpu_dec_gate    { clocks, "vpu_dec_gate",    gate_range(99),  vpu_bus_clk_root        };
};

