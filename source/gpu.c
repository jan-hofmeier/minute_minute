/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2023          Max Thomas <mtinc2@gmail.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "gpu.h"

#include "asic.h"
#include "latte.h"
#include "utils.h"
#include "dram.h"
#include "i2c.h"
#include "gfx.h"
#include "gpu_init.h"
#include <string.h>

void* gpu_tv_primary_surface_addr(void) {
    return (void*)(abif_gpu_read32(D1GRPH_PRIMARY_SURFACE_ADDRESS) & ~4);
}

void* gpu_drc_primary_surface_addr(void) {
    return (void*)(abif_gpu_read32(D2GRPH_PRIMARY_SURFACE_ADDRESS) & ~4);
}

void gpu_dump_dc_regs()
{
    for (int i = 0; i < 0x800; i += 4) {
        printf("%04x: %08x\n", 0x6100 + i, abif_gpu_read32(0x6100 + i)); 
    }
    for (int i = 0; i < 0x800; i += 4) {
        printf("%04x: %08x\n", 0x6900 + i, abif_gpu_read32(0x6900 + i)); 
    }
}

void gpu_test(void) {
#if 0
    // 01200000 when working, 01000004 when JTAG fuses unloaded
    //printf("UVD idk %08x\n", abif_gpu_read32(0x3D57 * 4)); 
#endif

    //gpu_dump_dc_regs();
#if 1
    gpu_display_init();
    //gpu_dump_dc_regs();
#endif
}

bsp_pll_cfg vi1_pllcfg = {0,  1,  1,  0,  0,  0, 0, 0xC, 0x1F4,  0, 0xC, 0xC, 0xC,  0,  0,  0, 0x3D, 0x1C}; 
bsp_pll_cfg vi2_pllcfg = {0,  1,  1,  0,  0,  0,  0,  0, 0x2C,  0, 0x2C,  4, 0x2C,  0,  0,  0,  4,  0};
bsp_pll_cfg upll_cfg = {0, 1, 1, 0, 0, 0, 1, 0, 0x48, 0, 8, 0xA, 0x54, 0x1C2, 0, 0xd, 0x8, 0x0};

int pll_vi1_shutdown()
{
    u16 v0;

    v0 = abif_cpl_tr_read16(0x14);
    if (v0 & 0x8000)
    {
        v0 &= ~0x8000;
        abif_cpl_tr_write16(0x14, v0);
        udelay(5);
    }
    abif_cpl_tr_write16(0x14, v0 & 0xBFFF);
    //abif_cpl_tr_write16(2, 0);
    return 0;
}

int pll_vi1_shutdown_alt()
{
    u16 v0;

    v0 = abif_cpl_tr_read16(0x14);
    if (v0 & 0x8000)
    {
        v0 &= ~0x8000;
        abif_cpl_tr_write16(0x14, v0);
        udelay(5);
    }
    abif_cpl_tr_write16(0x14, v0 & 0xBFFF);
    abif_cpl_tr_write16(2, 0);
    return 0;
}

int pll_vi2_shutdown()
{
    u16 v0;

    v0 = abif_cpl_tr_read16(0x34);
    if (v0 & 0x8000)
    {
        v0 &= ~0x8000;
        abif_cpl_tr_write16(0x34, v0);
        udelay(5);
    }
    abif_cpl_tr_write16(0x34, v0 & 0xBFFF);
    return 0;
}

int pll_vi1_write(bsp_pll_cfg *pCfg)
{
    u16 v4; // r1
    u16 v5; // r3
    u16 v7; // r1
    u16 v8; // r3
    u16 v10; // r1
    u16 v11; // r3
    u16 v13; // r1
    u16 v14; // r3
    u16 v16; // r1
    u32 v17; // r3

    //pll_vi1_shutdown(); // TODO: remove the 2 write?
    udelay(10);
    abif_cpl_tr_write16(2, pCfg->options & 0x1E); // 0x1C

    abif_cpl_tr_write16(0x10, pCfg->clkR | 0x8000 | (u16)(pCfg->clkO0Div << 6)); // 0x30c

    v4 = pCfg->bypVco ? 0x1000 : 0;
    v5 = pCfg->bypOut ? 0x2000 : 0;
    abif_cpl_tr_write16(0x12, (u16)v4 | (u16)(v5 | pCfg->clkFMsb)); // 0x1F4

    v7 = pCfg->satEn ? 0x800 : 0;
    v8 = pCfg->fastEn ? 0x1000 : 0;
    abif_cpl_tr_write16(0x14, (u16)v7 | (u16)(v8 | pCfg->clkO2Div)); // 0000c80c
    abif_cpl_tr_write16(0x16, pCfg->clkO1Div); // 0000000c
    abif_cpl_tr_write16(0x18, pCfg->clkVLsb); // 00000000

    v10 = pCfg->ssEn ? 0x400 : 0;
    v11 = pCfg->dithEn ? 0x800 : 0;
    abif_cpl_tr_write16(0x1A, (u16)v10 | (u16)(v11 | pCfg->clkVMsb)); // 00000800
    abif_cpl_tr_write16(0x1C, pCfg->clkS); // 00000000
    abif_cpl_tr_write16(0x1E, pCfg->bwAdj); // 0000003d
    abif_cpl_tr_write16(0x20, pCfg->clkFLsb); // 0
    udelay(5);

    v13 = pCfg->satEn ? 0x4800 : 0x4000;
    v14 = pCfg->fastEn ? 0x1000 : 0x0;
    abif_cpl_tr_write16(0x14, (u16)v13 | (u16)(v14 | pCfg->clkO2Div));
    udelay(50);
    
    v16 = pCfg->satEn ? 0xC800 : 0xC000;
    v17 = pCfg->fastEn ? 0x1000 : 0;
    abif_cpl_tr_write16(0x14, (u16)v16 | (u16)(v17 | pCfg->clkO2Div));
    udelay(50);

    return 0;
}

int pll_vi2_write(bsp_pll_cfg *pCfg)
{
    u16 v4; // r1
    u16 v5; // r3
    u16 v7; // r1
    u16 v8; // r3
    u16 v10; // r1
    u16 v11; // r3
    u16 v13; // r1
    u16 v14; // r3
    u16 v16; // r1
    u32 v17; // r3

    //pll_vi2_shutdown();
    udelay(10);

    abif_cpl_tr_write16(0x30, pCfg->clkR | 0x8000 | (u16)(pCfg->clkO0Div << 6)); // 00000b00

    v4 = pCfg->bypVco ? 0x1000 : 0;
    v5 = pCfg->bypOut ? 0x2000 : 0;
    abif_cpl_tr_write16(0x32, (u16)v4 | (u16)(v5 | pCfg->clkFMsb)); // 0000002c
    
    v7 = pCfg->satEn ? 0x800 : 0;
    v8 = pCfg->fastEn ? 0x1000 : 0;
    abif_cpl_tr_write16(0x34, (u16)v7 | (u16)(v8 | pCfg->clkO2Div)); // 0000c82c
    abif_cpl_tr_write16(0x36, pCfg->clkO1Div); // 4
    abif_cpl_tr_write16(0x24, pCfg->clkVLsb); // 0

    v10 = pCfg->ssEn ? 0x400 : 0;
    v11 = pCfg->dithEn ? 0x800 : 0;
    abif_cpl_tr_write16(0x26, (u16)v10 | (u16)(v11 | pCfg->clkVMsb)); // 0x800
    abif_cpl_tr_write16(0x28, pCfg->clkS); // 0
    abif_cpl_tr_write16(0x2A, pCfg->bwAdj); // 4
    abif_cpl_tr_write16(0x2C, pCfg->clkFLsb); // 0
    udelay(5);

    v13 = pCfg->satEn ? 0x4800 : 0x4000;
    v14 = pCfg->fastEn ? 0x1000 : 0x0;
    abif_cpl_tr_write16(0x34, (u16)v13 | (u16)(v14 | pCfg->clkO2Div)); // 0000c82c
    udelay(50);
    
    v16 = pCfg->satEn ? 0xC800 : 0xC000;
    v17 = pCfg->fastEn ? 0x1000 : 0;
    abif_cpl_tr_write16(0x34, (u16)v16 | (u16)(v17 | pCfg->clkO2Div));
    udelay(50);

    return 0;
}

void gpu_switch_endianness(void) {
    if (read16(MEM_GPU_ENDIANNESS) & 3 == 1)
        return;

    abif_gpu_write32(0x8020, 0xFFFFFFFF);
    abif_gpu_write32(0xe60, 0x820);
    write16(MEM_GPU_ENDIANNESS, 1);
    udelay(100);
    abif_gpu_write32(0xe60, 0);
    abif_gpu_write32(0x8020, 0);
    udelay(100);
}

void gpu_do_init_list(gpu_init_entry_t* paEntries, u32 len) {
    for (int i = 0; i < len; i++) {
        gpu_init_entry_t* entry = &paEntries[i];

        printf("%04x: %08x\n", entry->addr, abif_gpu_read32(entry->addr));

        if (entry->clear_bits) {
            u32 val = abif_gpu_read32(entry->addr);
            val &= ~entry->clear_bits;
            val |= entry->set_bits;
            abif_gpu_write32(entry->addr, val);
            //abif_gpu_mask32(entry->addr, entry->clear_bits, entry->set_bits);
        }
        else {
            abif_gpu_write32(entry->addr, entry->set_bits);
        }

        /*if (gpu_tv_primary_surface_addr()) {
            printf("%u GPU addr: %08x\n", i, gpu_tv_primary_surface_addr());
        }*/
        

        if (entry->usec_delay) {
            udelay(entry->usec_delay);
        }
    }
}

void gpu_do_ave_list(ave_init_entry_t* paEntries, u32 len) {
    for (int i = 0; i < len; i++) {
        ave_init_entry_t* entry = &paEntries[i];

        u8 tmp[2];
        tmp[0] = entry->reg_idx;
        tmp[1] = entry->value;
        ave_i2c_write(entry->addr, tmp, 2);

        if (entry->usec_delay) {
            udelay(entry->usec_delay);
        }
    }
}

int BSP_60XeDataStreaming_write(int val)
{
    int v4; // r6
    int v5; // r4
    unsigned int v6; // r4
    int v7; // r3
    int v9; // [sp+0h] [bp-18h] BYREF

    v9 = latte_get_hw_version();
    if ( (v9 & 0xF000000) != 0 )
    {
        set32(LT_60XE_CFG, 0x1000);
        v5 = read32(LT_60XE_CFG);
        udelay(10);
        v6 = v5 & ~8u;
        if (val == 1)
            v7 = 0;
        else
            v7 = 8;
        write32(LT_60XE_CFG, (v7 | v6) & ~0x1000);
    }
    else
    {
        return 1024;
    }
    return 0;
}

int gpu_idk_upll()
{
  if ( (read32(0xD8005E0) & 4) == 0 )
  {
    set32(0xD8005E0, 4);
    udelay(10);
  }
  abif_gpu_mask32(0x5930, 0x1FF, 0x49);
  abif_gpu_mask32(0x5938, 0x1FF, 0x49);
  set32(0xD800628, 1);
  return 0;
}

int upll_write(bsp_pll_cfg *pCfg)
{
    unsigned int v2; // lr
    unsigned int v3; // r1
    unsigned int v4; // r12
    unsigned int v5; // r1
    unsigned int v6; // lr
    unsigned int v7; // r1

    abif_cpl_ct_write32(0x888u, 0x2001u);
    abif_cpl_ct_write32(0x878u, pCfg->clkR | (pCfg->clkFMsb << 6) | 0x18000000 | (pCfg->clkO0Div << 18));
    udelay(5);
    abif_cpl_ct_write32(0x878u, pCfg->clkR | (pCfg->clkFMsb << 6) | 0x98000000 | (pCfg->clkO0Div << 18));
    udelay(10);
    abif_cpl_tr_write16(0, pCfg->clkO2Div);
    v2 = pCfg->satEn ? 0x8000000 : 0;
    v3 = pCfg->fastEn ? 0x10000000 : 0;
    abif_cpl_ct_write32(0x87Cu, v2 | v3 | pCfg->clkO1Div | (pCfg->clkFLsb << 9));
    v4 = pCfg->ssEn ? 0x4000000 : 0;
    v5 = pCfg->dithEn ? 0x8000000 : 0;
    abif_cpl_ct_write32(0x880u, v4 | v5 | pCfg->clkVLsb | (pCfg->clkVMsb << 16));
    abif_cpl_ct_write32(0x884u, pCfg->clkS | (pCfg->bwAdj << 12));
    udelay(5);
    abif_cpl_ct_write32(0x878u, pCfg->clkR | (pCfg->clkFMsb << 6) | 0x18000000 | (pCfg->clkO0Div << 18));
    udelay(200);
    v6 = pCfg->bypVco ? 0x8000000 : 0;
    v7 = pCfg->bypOut ? 0x10000000 : 0;
    abif_cpl_ct_write32(0x878u, v6 | v7 | pCfg->clkR | (pCfg->clkFMsb << 6) | (pCfg->clkO0Div << 18));
    abif_cpl_ct_write32(0x888u, 0x4002u);

    return 0;
}

int upll_read(bsp_pll_cfg *pOut)
{
    int v4; // r0
    int v5; // r2
    int v6; // r0
    unsigned int v7; // r2
    int v8; // r3
    unsigned int v9; // r1
    unsigned int v10; // r0
    int v11; // r0
    unsigned int v12; // r3
    unsigned int v13; // r1
    unsigned int v14; // r12
    int v15; // r0
    int v17; // r0
    int result; // r0

    memset(pOut, 0, sizeof(bsp_pll_cfg));

    v4 = abif_cpl_ct_read32(0x878u);
    pOut->clkR = v4 & 0x3F;
    pOut->bypVco = !!(v4 & 0x8000000u);
    pOut->bypOut = !!(v4 & 0x10000000u);
    pOut->clkFMsb = (v4 >> 6) & 0x3FFF;
    pOut->clkO0Div = (v4 >> 18) & 0x1FF;
    v6 = abif_cpl_ct_read32(0x87Cu);
    pOut->clkFLsb = (v6 >> 9) & 0x3FFF;
    pOut->clkO1Div = v6 & 0x1FF;
    pOut->satEn = !!(v6 & 0x8000000);
    pOut->fastEn = !!(v6 & 0x10000000);
    v11 = abif_cpl_ct_read32(0x880u);
    pOut->clkVMsb = (v11 >> 16) & 0x3FFF;
    pOut->clkVLsb = v11 & 0xFFFF;
    pOut->ssEn = !!(v11 & 0x4000000u);
    pOut->dithEn = !!(v11 & 0x8000000);
    v15 = abif_cpl_ct_read32(0x884u);
    pOut->bwAdj = (v15 & 0xFFF000u) >> 12;
    pOut->clkO2Div = abif_cpl_tr_read16(0) & 0x1FF;
    v17 = abif_cpl_ct_read32(0x878u);
    if ( (v17 & 0x20000000) != 0
        || (v17 & 0x10000000) != 0
        || (v17 & 0x8000000u) >> 27
        || (abif_cpl_ct_read32(0x888u) & 0xE000) != 0x4000 && (abif_cpl_ct_read32(0x888u) & 7) != 2 )
    {
        return 0;
    }

    pOut->operational = 1;
    return 0;
}

void upll_init()
{
    /*int v0; // r5
    int v2; // r4
    int v3; // r0
    bsp_pll_cfg cfg; // [sp+2h] [bp-46h] BYREF
    int dram_operational; // [sp+38h] [bp-10h] BYREF

    v0 = BSP_check_dram_operational(&dram_operational);
    if ( !v0 )
    {
        if ( dram_operational == 2 )
        {
            v2 = BSP_vi1_pll_shutdown();
            v3 = BSP_vi2_pll_shutdown();
            MEMORY[0xD8005E0] &= ~4u;
            return v2 | v3;
        }
        else
        {
            bsp_memset(&cfg.fastEn, 0, 0x36u);
            v0 = BSP_gpu_related();
            BSP_upll_read(0, 0, &cfg);
            if ( BSP_memcmp(&BSP_upll_config, &cfg, 54) )
                return v0 | BSP_upll_write_(&BSP_upll_config);
        }
    }
    return v0;*/

    bsp_pll_cfg cfg;
    memset(&cfg, 0, sizeof(bsp_pll_cfg));
    gpu_idk_upll();
    upll_read(&cfg);
    if (memcmp(&upll_cfg, &cfg, sizeof(bsp_pll_cfg))) {
        upll_write(&upll_cfg);
    }
}

//abifr 0x01000002

void gpu_display_init(void) {
    //gpu_switch_endianness();
    ave_i2c_init(400000, 0);

    //pll_vi1_shutdown_alt();
    //pll_vi2_shutdown();

    pll_vi1_write(&vi1_pllcfg);
    pll_vi2_write(&vi2_pllcfg);

    //gpu_do_init_list(gpu_init_entries_A, NUM_GPU_ENTRIES_A);
    //gpu_do_ave_list(ave_init_entries_A, NUM_AVE_ENTRIES_A);

    //gpu_do_init_list(gpu_init_entries_B, NUM_GPU_ENTRIES_B);
    //gpu_do_ave_list(ave_init_entries_B, NUM_AVE_ENTRIES_B);

    // messes up DRC?
    gpu_do_init_list(gpu_init_entries_C, NUM_GPU_ENTRIES_C);
    gpu_do_ave_list(ave_init_entries_C, NUM_AVE_ENTRIES_C);

    printf("GPU TV addr: %08x\n", gpu_tv_primary_surface_addr());
    printf("GPU DRC addr: %08x\n", gpu_drc_primary_surface_addr());

    abif_gpu_write32(D1GRPH_PRIMARY_SURFACE_ADDRESS, FB_TV_ADDR);
    abif_gpu_write32(D2GRPH_PRIMARY_SURFACE_ADDRESS, FB_DRC_ADDR);

    // HACK: I can't get the endianness swap to work :/
    if (read16(MEM_GPU_ENDIANNESS) & 3 != 1)
        abif_gpu_write32(D1GRPH_SWAP_CNTL, 0x222);

    //BSP_60XeDataStreaming_write(1);
    upll_init();
}

void gpu_cleanup()
{
    abif_gpu_write32(0x60e0, 0x0);
    abif_gpu_write32(0x898, 0xFFFFFFFF);

    // HACK: I can't get the endianness swap to work :/
    if (read16(MEM_GPU_ENDIANNESS) & 3 != 1)
        abif_gpu_write32(D1GRPH_SWAP_CNTL, 0x220);
}