/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _DT_BINDINGS_CLK_MSM_GCC_8974_H
#define _DT_BINDINGS_CLK_MSM_GCC_8974_H

#define GPLL0							0
#define GPLL0_VOTE						1
#define CONFIG_NOC_CLK_SRC					2
#define GPLL2							3
#define GPLL2_VOTE						4
#define GPLL3							5
#define GPLL3_VOTE						6
#define PERIPH_NOC_CLK_SRC					7
#define BLSP_UART_SIM_CLK_SRC					8
#define QDSS_TSCTR_CLK_SRC					9
#define BIMC_DDR_CLK_SRC					10
#define SYSTEM_NOC_CLK_SRC					11
#define GPLL1							12
#define GPLL1_VOTE						13
#define RPM_CLK_SRC						14
#define GCC_BIMC_CLK						15
#define BIMC_DDR_CPLL0_ROOT_CLK_SRC				16
#define KPSS_AHB_CLK_SRC					17
#define QDSS_AT_CLK_SRC						18
#define USB30_MASTER_CLK_SRC					19
#define BIMC_DDR_CPLL1_ROOT_CLK_SRC				20
#define QDSS_STM_CLK_SRC					21
#define ACC_CLK_SRC						22
#define SEC_CTRL_CLK_SRC					23
#define BLSP1_QUP1_I2C_APPS_CLK_SRC				24
#define BLSP1_QUP1_SPI_APPS_CLK_SRC				25
#define BLSP1_QUP2_I2C_APPS_CLK_SRC				26
#define BLSP1_QUP2_SPI_APPS_CLK_SRC				27
#define BLSP1_QUP3_I2C_APPS_CLK_SRC				28
#define BLSP1_QUP3_SPI_APPS_CLK_SRC				29
#define BLSP1_QUP4_I2C_APPS_CLK_SRC				30
#define BLSP1_QUP4_SPI_APPS_CLK_SRC				31
#define BLSP1_QUP5_I2C_APPS_CLK_SRC				32
#define BLSP1_QUP5_SPI_APPS_CLK_SRC				33
#define BLSP1_QUP6_I2C_APPS_CLK_SRC				34
#define BLSP1_QUP6_SPI_APPS_CLK_SRC				35
#define BLSP1_UART1_APPS_CLK_SRC				36
#define BLSP1_UART2_APPS_CLK_SRC				37
#define BLSP1_UART3_APPS_CLK_SRC				38
#define BLSP1_UART4_APPS_CLK_SRC				39
#define BLSP1_UART5_APPS_CLK_SRC				40
#define BLSP1_UART6_APPS_CLK_SRC				41
#define BLSP2_QUP1_I2C_APPS_CLK_SRC				42
#define BLSP2_QUP1_SPI_APPS_CLK_SRC				43
#define BLSP2_QUP2_I2C_APPS_CLK_SRC				44
#define BLSP2_QUP2_SPI_APPS_CLK_SRC				45
#define BLSP2_QUP3_I2C_APPS_CLK_SRC				46
#define BLSP2_QUP3_SPI_APPS_CLK_SRC				47
#define BLSP2_QUP4_I2C_APPS_CLK_SRC				48
#define BLSP2_QUP4_SPI_APPS_CLK_SRC				49
#define BLSP2_QUP5_I2C_APPS_CLK_SRC				50
#define BLSP2_QUP5_SPI_APPS_CLK_SRC				51
#define BLSP2_QUP6_I2C_APPS_CLK_SRC				52
#define BLSP2_QUP6_SPI_APPS_CLK_SRC				53
#define BLSP2_UART1_APPS_CLK_SRC				54
#define BLSP2_UART2_APPS_CLK_SRC				55
#define BLSP2_UART3_APPS_CLK_SRC				56
#define BLSP2_UART4_APPS_CLK_SRC				57
#define BLSP2_UART5_APPS_CLK_SRC				58
#define BLSP2_UART6_APPS_CLK_SRC				59
#define CE1_CLK_SRC						60
#define CE2_CLK_SRC						61
#define GP1_CLK_SRC						62
#define GP2_CLK_SRC						63
#define GP3_CLK_SRC						64
#define PDM2_CLK_SRC						65
#define QDSS_TRACECLKIN_CLK_SRC					66
#define RBCPR_CLK_SRC						67
#define SDCC1_APPS_CLK_SRC					68
#define SDCC2_APPS_CLK_SRC					69
#define SDCC3_APPS_CLK_SRC					70
#define SDCC4_APPS_CLK_SRC					71
#define SPMI_AHB_CLK_SRC					72
#define SPMI_SER_CLK_SRC					73
#define TSIF_REF_CLK_SRC					74
#define USB30_MOCK_UTMI_CLK_SRC					75
#define USB_HS_SYSTEM_CLK_SRC					76
#define USB_HSIC_CLK_SRC					77
#define USB_HSIC_IO_CAL_CLK_SRC					78
#define USB_HSIC_SYSTEM_CLK_SRC					79
#define GCC_BAM_DMA_AHB_CLK					80
#define GCC_BAM_DMA_INACTIVITY_TIMERS_CLK			81
#define GCC_BIMC_CFG_AHB_CLK					82
#define GCC_BIMC_KPSS_AXI_CLK					83
#define GCC_BIMC_SLEEP_CLK					84
#define GCC_BIMC_SYSNOC_AXI_CLK					85
#define GCC_BIMC_XO_CLK						86
#define GCC_BLSP1_AHB_CLK					87
#define GCC_BLSP1_SLEEP_CLK					88
#define GCC_BLSP1_QUP1_I2C_APPS_CLK				89
#define GCC_BLSP1_QUP1_SPI_APPS_CLK				90
#define GCC_BLSP1_QUP2_I2C_APPS_CLK				91
#define GCC_BLSP1_QUP2_SPI_APPS_CLK				92
#define GCC_BLSP1_QUP3_I2C_APPS_CLK				93
#define GCC_BLSP1_QUP3_SPI_APPS_CLK				94
#define GCC_BLSP1_QUP4_I2C_APPS_CLK				95
#define GCC_BLSP1_QUP4_SPI_APPS_CLK				96
#define GCC_BLSP1_QUP5_I2C_APPS_CLK				97
#define GCC_BLSP1_QUP5_SPI_APPS_CLK				98
#define GCC_BLSP1_QUP6_I2C_APPS_CLK				99
#define GCC_BLSP1_QUP6_SPI_APPS_CLK				100
#define GCC_BLSP1_UART1_APPS_CLK				101
#define GCC_BLSP1_UART1_SIM_CLK					102
#define GCC_BLSP1_UART2_APPS_CLK				103
#define GCC_BLSP1_UART2_SIM_CLK					104
#define GCC_BLSP1_UART3_APPS_CLK				105
#define GCC_BLSP1_UART3_SIM_CLK					106
#define GCC_BLSP1_UART4_APPS_CLK				107
#define GCC_BLSP1_UART4_SIM_CLK					108
#define GCC_BLSP1_UART5_APPS_CLK				109
#define GCC_BLSP1_UART5_SIM_CLK					110
#define GCC_BLSP1_UART6_APPS_CLK				111
#define GCC_BLSP1_UART6_SIM_CLK					112
#define GCC_BLSP2_AHB_CLK					113
#define GCC_BLSP2_SLEEP_CLK					114
#define GCC_BLSP2_QUP1_I2C_APPS_CLK				115
#define GCC_BLSP2_QUP1_SPI_APPS_CLK				116
#define GCC_BLSP2_QUP2_I2C_APPS_CLK				117
#define GCC_BLSP2_QUP2_SPI_APPS_CLK				118
#define GCC_BLSP2_QUP3_I2C_APPS_CLK				119
#define GCC_BLSP2_QUP3_SPI_APPS_CLK				120
#define GCC_BLSP2_QUP4_I2C_APPS_CLK				121
#define GCC_BLSP2_QUP4_SPI_APPS_CLK				122
#define GCC_BLSP2_QUP5_I2C_APPS_CLK				123
#define GCC_BLSP2_QUP5_SPI_APPS_CLK				124
#define GCC_BLSP2_QUP6_I2C_APPS_CLK				125
#define GCC_BLSP2_QUP6_SPI_APPS_CLK				126
#define GCC_BLSP2_UART1_APPS_CLK				127
#define GCC_BLSP2_UART1_SIM_CLK					128
#define GCC_BLSP2_UART2_APPS_CLK				129
#define GCC_BLSP2_UART2_SIM_CLK					130
#define GCC_BLSP2_UART3_APPS_CLK				131
#define GCC_BLSP2_UART3_SIM_CLK					132
#define GCC_BLSP2_UART4_APPS_CLK				133
#define GCC_BLSP2_UART4_SIM_CLK					134
#define GCC_BLSP2_UART5_APPS_CLK				135
#define GCC_BLSP2_UART5_SIM_CLK					136
#define GCC_BLSP2_UART6_APPS_CLK				137
#define GCC_BLSP2_UART6_SIM_CLK					138
#define GCC_BOOT_ROM_AHB_CLK					139
#define GCC_CE1_AHB_CLK						140
#define GCC_CE1_AXI_CLK						141
#define GCC_CE1_CLK						142
#define GCC_CE2_AHB_CLK						143
#define GCC_CE2_AXI_CLK						144
#define GCC_CE2_CLK						145
#define GCC_CNOC_BUS_TIMEOUT0_AHB_CLK				146
#define GCC_CNOC_BUS_TIMEOUT1_AHB_CLK				147
#define GCC_CNOC_BUS_TIMEOUT2_AHB_CLK				148
#define GCC_CNOC_BUS_TIMEOUT3_AHB_CLK				149
#define GCC_CNOC_BUS_TIMEOUT4_AHB_CLK				150
#define GCC_CNOC_BUS_TIMEOUT5_AHB_CLK				151
#define GCC_CNOC_BUS_TIMEOUT6_AHB_CLK				152
#define GCC_CFG_NOC_AHB_CLK					153
#define GCC_CFG_NOC_DDR_CFG_CLK					154
#define GCC_CFG_NOC_RPM_AHB_CLK					155
#define GCC_BIMC_DDR_CPLL0_CLK					156
#define GCC_BIMC_DDR_CPLL1_CLK					157
#define GCC_DDR_DIM_CFG_CLK					158
#define GCC_DDR_DIM_SLEEP_CLK					159
#define GCC_DEHR_CLK						160
#define GCC_AHB_CLK						161
#define GCC_IM_SLEEP_CLK					162
#define GCC_XO_CLK						163
#define GCC_XO_DIV4_CLK						164
#define GCC_GP1_CLK						165
#define GCC_GP2_CLK						166
#define GCC_GP3_CLK						167
#define GCC_IMEM_AXI_CLK					168
#define GCC_IMEM_CFG_AHB_CLK					169
#define GCC_KPSS_AHB_CLK					170
#define GCC_KPSS_AXI_CLK					171
#define GCC_LPASS_Q6_AXI_CLK					172
#define GCC_MMSS_NOC_AT_CLK					173
#define GCC_MMSS_NOC_CFG_AHB_CLK				174
#define GCC_OCMEM_NOC_CFG_AHB_CLK				175
#define GCC_OCMEM_SYS_NOC_AXI_CLK				176
#define GCC_MPM_AHB_CLK						177
#define GCC_MSG_RAM_AHB_CLK					178
#define GCC_MSS_CFG_AHB_CLK					179
#define GCC_MSS_Q6_BIMC_AXI_CLK					180
#define GCC_NOC_CONF_XPU_AHB_CLK				181
#define GCC_PDM2_CLK						182
#define GCC_PDM_AHB_CLK						183
#define GCC_PDM_XO4_CLK						184
#define GCC_PERIPH_NOC_AHB_CLK					185
#define GCC_PERIPH_NOC_AT_CLK					186
#define GCC_PERIPH_NOC_CFG_AHB_CLK				187
#define GCC_PERIPH_NOC_MPU_CFG_AHB_CLK				188
#define GCC_PERIPH_XPU_AHB_CLK					189
#define GCC_PNOC_BUS_TIMEOUT0_AHB_CLK				190
#define GCC_PNOC_BUS_TIMEOUT1_AHB_CLK				191
#define GCC_PNOC_BUS_TIMEOUT2_AHB_CLK				192
#define GCC_PNOC_BUS_TIMEOUT3_AHB_CLK				193
#define GCC_PNOC_BUS_TIMEOUT4_AHB_CLK				194
#define GCC_PRNG_AHB_CLK					195
#define GCC_QDSS_AT_CLK						196
#define GCC_QDSS_CFG_AHB_CLK					197
#define GCC_QDSS_DAP_AHB_CLK					198
#define GCC_QDSS_DAP_CLK					199
#define GCC_QDSS_ETR_USB_CLK					200
#define GCC_QDSS_STM_CLK					201
#define GCC_QDSS_TRACECLKIN_CLK					202
#define GCC_QDSS_TSCTR_DIV16_CLK				203
#define GCC_QDSS_TSCTR_DIV2_CLK					204
#define GCC_QDSS_TSCTR_DIV3_CLK					205
#define GCC_QDSS_TSCTR_DIV4_CLK					206
#define GCC_QDSS_TSCTR_DIV8_CLK					207
#define GCC_QDSS_RBCPR_XPU_AHB_CLK				208
#define GCC_RBCPR_AHB_CLK					209
#define GCC_RBCPR_CLK						210
#define GCC_RPM_BUS_AHB_CLK					211
#define GCC_RPM_PROC_HCLK					212
#define GCC_RPM_SLEEP_CLK					213
#define GCC_RPM_TIMER_CLK					214
#define GCC_SDCC1_AHB_CLK					215
#define GCC_SDCC1_APPS_CLK					216
#define GCC_SDCC1_INACTIVITY_TIMERS_CLK				217
#define GCC_SDCC2_AHB_CLK					218
#define GCC_SDCC2_APPS_CLK					219
#define GCC_SDCC2_INACTIVITY_TIMERS_CLK				220
#define GCC_SDCC3_AHB_CLK					221
#define GCC_SDCC3_APPS_CLK					222
#define GCC_SDCC3_INACTIVITY_TIMERS_CLK				223
#define GCC_SDCC4_AHB_CLK					224
#define GCC_SDCC4_APPS_CLK					225
#define GCC_SDCC4_INACTIVITY_TIMERS_CLK				226
#define GCC_SEC_CTRL_ACC_CLK					227
#define GCC_SEC_CTRL_AHB_CLK					228
#define GCC_SEC_CTRL_BOOT_ROM_PATCH_CLK				229
#define GCC_SEC_CTRL_CLK					230
#define GCC_SEC_CTRL_SENSE_CLK					231
#define GCC_SNOC_BUS_TIMEOUT0_AHB_CLK				232
#define GCC_SNOC_BUS_TIMEOUT2_AHB_CLK				233
#define GCC_SPDM_BIMC_CY_CLK					234
#define GCC_SPDM_CFG_AHB_CLK					235
#define GCC_SPDM_DEBUG_CY_CLK					236
#define GCC_SPDM_FF_CLK						237
#define GCC_SPDM_MSTR_AHB_CLK					238
#define GCC_SPDM_PNOC_CY_CLK					239
#define GCC_SPDM_RPM_CY_CLK					240
#define GCC_SPDM_SNOC_CY_CLK					241
#define GCC_SPMI_AHB_CLK					242
#define GCC_SPMI_CNOC_AHB_CLK					243
#define GCC_SPMI_SER_CLK					244
#define GCC_SNOC_CNOC_AHB_CLK					245
#define GCC_SNOC_PNOC_AHB_CLK					246
#define GCC_SYS_NOC_AT_CLK					247
#define GCC_SYS_NOC_AXI_CLK					248
#define GCC_SYS_NOC_KPSS_AHB_CLK				249
#define GCC_SYS_NOC_QDSS_STM_AXI_CLK				250
#define GCC_SYS_NOC_USB3_AXI_CLK				251
#define GCC_TCSR_AHB_CLK					252
#define GCC_TLMM_AHB_CLK					253
#define GCC_TLMM_CLK						254
#define GCC_TSIF_AHB_CLK					255
#define GCC_TSIF_INACTIVITY_TIMERS_CLK				256
#define GCC_TSIF_REF_CLK					257
#define GCC_USB2A_PHY_SLEEP_CLK					258
#define GCC_USB2B_PHY_SLEEP_CLK					259
#define GCC_USB30_MASTER_CLK					260
#define GCC_USB30_MOCK_UTMI_CLK					261
#define GCC_USB30_SLEEP_CLK					262
#define GCC_USB_HS_AHB_CLK					263
#define GCC_USB_HS_INACTIVITY_TIMERS_CLK			264
#define GCC_USB_HS_SYSTEM_CLK					265
#define GCC_USB_HSIC_AHB_CLK					266
#define GCC_USB_HSIC_CLK					267
#define GCC_USB_HSIC_IO_CAL_CLK					268
#define GCC_USB_HSIC_IO_CAL_SLEEP_CLK				269
#define GCC_USB_HSIC_SYSTEM_CLK					270
#define GCC_WCSS_GPLL1_CLK_SRC					271
#define GCC_MMSS_GPLL0_CLK_SRC					272
#define GCC_LPASS_GPLL0_CLK_SRC					273
#define GCC_WCSS_GPLL1_CLK_SRC_SLEEP_ENA			274
#define GCC_MMSS_GPLL0_CLK_SRC_SLEEP_ENA			275
#define GCC_LPASS_GPLL0_CLK_SRC_SLEEP_ENA			276
#define GCC_IMEM_AXI_CLK_SLEEP_ENA				277
#define GCC_SYS_NOC_KPSS_AHB_CLK_SLEEP_ENA			278
#define GCC_BIMC_KPSS_AXI_CLK_SLEEP_ENA				279
#define GCC_KPSS_AHB_CLK_SLEEP_ENA				280
#define GCC_KPSS_AXI_CLK_SLEEP_ENA				281
#define GCC_MPM_AHB_CLK_SLEEP_ENA				282
#define GCC_OCMEM_SYS_NOC_AXI_CLK_SLEEP_ENA			283
#define GCC_BLSP1_AHB_CLK_SLEEP_ENA				284
#define GCC_BLSP1_SLEEP_CLK_SLEEP_ENA				285
#define GCC_BLSP2_AHB_CLK_SLEEP_ENA				286
#define GCC_BLSP2_SLEEP_CLK_SLEEP_ENA				287
#define GCC_PRNG_AHB_CLK_SLEEP_ENA				288
#define GCC_BAM_DMA_AHB_CLK_SLEEP_ENA				289
#define GCC_BAM_DMA_INACTIVITY_TIMERS_CLK_SLEEP_ENA		290
#define GCC_BOOT_ROM_AHB_CLK_SLEEP_ENA				291
#define GCC_MSG_RAM_AHB_CLK_SLEEP_ENA				292
#define GCC_TLMM_AHB_CLK_SLEEP_ENA				293
#define GCC_TLMM_CLK_SLEEP_ENA					294
#define GCC_SPMI_CNOC_AHB_CLK_SLEEP_ENA				295
#define GCC_CE1_CLK_SLEEP_ENA					296
#define GCC_CE1_AXI_CLK_SLEEP_ENA				297
#define GCC_CE1_AHB_CLK_SLEEP_ENA				298
#define GCC_CE2_CLK_SLEEP_ENA					299
#define GCC_CE2_AXI_CLK_SLEEP_ENA				300
#define GCC_CE2_AHB_CLK_SLEEP_ENA				301

#endif
