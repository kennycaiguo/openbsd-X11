/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atichip.c,v 1.1.2.1 1998/02/01 16:41:43 robin Exp $ */
/*
 * Copyright 1997,1998 by Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of Marc Aurele La France not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Marc Aurele La France makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as-is" without express or implied warranty.
 *
 * MARC AURELE LA FRANCE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL MARC AURELE LA FRANCE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "atichip.h"
#include "atiio.h"

/*
 * Chip-related definitions.
 */
CARD8 ATIChip = ATI_CHIP_NONE;
const char *ATIChipNames[] =
{
    "Unknown",
    "IBM VGA or compatible",
    "ATI 18800",
    "ATI 18800-1",
    "ATI 28800-2",
    "ATI 28800-4",
    "ATI 28800-5",
    "ATI 28800-6",
    "IBM 8514/A",
    "Chips & Technologies 82C480",
    "ATI 38800-1",
    "ATI 68800",
    "ATI 68800-3",
    "ATI 68800-6",
    "ATI 68800LX",
    "ATI 68800AX",
    "ATI 88800GX-C",
    "ATI 88800GX-D",
    "ATI 88800GX-E",
    "ATI 88800GX-F",
    "ATI 88800GX",
    "ATI 88800CX",
    "ATI 264CT",
    "ATI 264ET",
    "ATI 264VT",
    "ATI 3D Rage",
    "ATI 264VT-B",
    "ATI 3D Rage II",
    "ATI 264VT3",
    "ATI 3D Rage II+DVD",
    "ATI 3D Rage LT",
    "ATI 3D Rage Pro",
    "ATI unknown Mach64",
};

CARD16 ATIChipType = 0, ATIChipClass = 0, ATIChipRevision = 0;
CARD16 ATIChipVersion = 0, ATIChipFoundry = 0;
CARD8 ATIChipHasSUBSYS_CNTL = FALSE;
CARD8 ATIChipHasVGAWonder   = FALSE;
const char *ATIFoundryNames[] =
    { "SGS", "NEC", "KCS", "UMC", "4", "5", "6", "UMC" };

/*
 * ATIMach32ChipID --
 *
 * Set variables whose value is dependent upon an 68800's CHIP_ID register.
 */
void
ATIMach32ChipID(void)
{
    CARD16 IO_Value = inw(CHIP_ID);
    ATIChipType     = GetBits(IO_Value, CHIP_CODE_0 | CHIP_CODE_1);
    ATIChipClass    = GetBits(IO_Value, CHIP_CLASS);
    ATIChipRevision = GetBits(IO_Value, CHIP_REV);
    if (IO_Value == 0xFFFFU)
        IO_Value = 0;
    switch (GetBits(IO_Value, CHIP_CODE_0 | CHIP_CODE_1))
    {
        case 0x0000U:
            ATIChip = ATI_CHIP_68800_3;
            break;

        case 0x02F7U:
            ATIChip = ATI_CHIP_68800_6;
            break;

        case 0x0177U:
            ATIChip = ATI_CHIP_68800LX;
            break;

        case 0x0017U:
            ATIChip = ATI_CHIP_68800AX;
            break;

        default:
            ATIChip = ATI_CHIP_68800;
            break;
    }
}

/*
 * ATIMach64ChipID --
 *
 * Set variables whose value is dependent upon a Mach64's CONFIG_CHIP_ID
 * register.
 */
void
ATIMach64ChipID(const CARD16 ExpectedChipType)
{
    CARD32 IO_Value = inl(ATIIOPort(CONFIG_CHIP_ID));
    ATIChipType     = GetBits(IO_Value, 0xFFFFU);
    ATIChipClass    = GetBits(IO_Value, CFG_CHIP_CLASS);
    ATIChipRevision = GetBits(IO_Value, CFG_CHIP_REV);
    ATIChipVersion  = GetBits(IO_Value, CFG_CHIP_VERSION);
    ATIChipFoundry  = GetBits(IO_Value, CFG_CHIP_FOUNDRY);
    switch (ATIChipType)
    {
        case 0x00D7U:
            ATIChipType = 0x4758U;
        case 0x4758U:
            switch (ATIChipRevision)
            {
                case 0x00U:
                    ATIChip = ATI_CHIP_88800GXC;
                    break;

                case 0x01U:
                    ATIChip = ATI_CHIP_88800GXD;
                    break;

                case 0x02U:
                    ATIChip = ATI_CHIP_88800GXE;
                    break;

                case 0x03U:
                    ATIChip = ATI_CHIP_88800GXF;
                    break;

                default:
                    ATIChip = ATI_CHIP_88800GX;
                    break;
            }
            break;

        case 0x0057U:
            ATIChipType = 0x4358U;
        case 0x4358U:
            ATIChip = ATI_CHIP_88800CX;
            break;

        case 0x0053U:
            ATIChipType = 0x4354U;
        case 0x4354U:
            ATIChipRevision = GetBits(IO_Value, CFG_CHIP_REVISION);
            ATIChip = ATI_CHIP_264CT;
            break;

        case 0x0093U:
            ATIChipType = 0x4554U;
        case 0x4554U:
            ATIChipRevision = GetBits(IO_Value, CFG_CHIP_REVISION);
            ATIChip = ATI_CHIP_264ET;
            break;

        case 0x02B3U:
            ATIChipType = 0x5654U;
        case 0x5654U:
            ATIChipRevision = GetBits(IO_Value, CFG_CHIP_REVISION);
            ATIChip = ATI_CHIP_264VT;
            /* Some early GT's are detected as VT's */
            if (ExpectedChipType && (ATIChipType != ExpectedChipType))
            {
                if (ExpectedChipType == 0x4754U)
                    ATIChip = ATI_CHIP_264GT;
                else
                    ErrorF("Mach64 chip type probe discrepancy detected:\n"
                           " PCI=0x%04X;  CHIP_ID=0x%04X.\n",
                           ExpectedChipType, ATIChipType);
            }
            else if (ATIChipVersion)
                ATIChip = ATI_CHIP_264VTB;
            break;

        case 0x00D3U:
            ATIChipType = 0x4754U;
        case 0x4754U:
            ATIChipRevision = GetBits(IO_Value, CFG_CHIP_REVISION);
            if (!ATIChipVersion)
                ATIChip = ATI_CHIP_264GT;
            else
                ATIChip = ATI_CHIP_264GTB;
            break;

        case 0x02B4U:
            ATIChipType = 0x5655U;
        case 0x5655U:
            ATIChipRevision = GetBits(IO_Value, CFG_CHIP_REVISION);
            ATIChip = ATI_CHIP_264VT3;
            break;

        case 0x00D4U:
            ATIChipType = 0x4755U;
        case 0x4755U:
            ATIChipRevision = GetBits(IO_Value, CFG_CHIP_REVISION);
            ATIChip = ATI_CHIP_264GTDVD;
            break;

        case 0x0166U:
            ATIChipType = 0x4C47U;
        case 0x4C47U:
            ATIChipRevision = GetBits(IO_Value, CFG_CHIP_REVISION);
            ATIChip = ATI_CHIP_264LT;
            break;

        case 0x00C7U:
        case 0x00C9U:
        case 0x00CEU:
        case 0x00CFU:
        case 0x00D0U:
            ATIChipType = 0x4750U;
        case 0x4742U:
        case 0x4744U:
        case 0x4749U:
        case 0x4750U:
        case 0x4751U:
            ATIChipRevision = GetBits(IO_Value, CFG_CHIP_REVISION);
            ATIChip = ATI_CHIP_264GT3;
            break;

        default:
            ATIChip = ATI_CHIP_Mach64;
            break;
    }
}