/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atiprint.c,v 1.1.2.2 1998/02/27 03:07:51 dawes Exp $ */
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

#include "atiadapter.h"
#include "atichip.h"
#include "atidac.h"
#include "atidepth.h"
#include "atiio.h"
#include "atiprint.h"
#include "atividmem.h"
#include "xf86Priv.h"

/*
 * Define a table to map mode flag values to XF86Config tokens.
 */
typedef struct
{
    int flag;
    char * token;
} TokenTabRec, *TokenTabPtr;

static TokenTabRec TokenTab[] =
{
    {V_PHSYNC,    "+hsync"},
    {V_NHSYNC,    "-hsync"},
    {V_PVSYNC,    "+vsync"},
    {V_NVSYNC,    "-vsync"},
    {V_PCSYNC,    "+csync"},
    {V_NCSYNC,    "-csync"},
    {V_INTERLACE, "interlace"},
    {V_DBLSCAN,   "doublescan"},
    {V_CSYNC,     "composite"},
    {0,           NULL}
};

/*
 * ATIPrintBIOS --
 *
 * Display part of the video BIOS when the server is invoked with -verbose.
 */
void
ATIPrintBIOS(const CARD8 * BIOS,
             const unsigned int Start, const unsigned int End)
{
    unsigned int Index = Start & ~(16U - 1U);

    ErrorF("\n BIOS data at 0x%08X:", Start + vga256InfoRec.BIOSbase);

    for (;  Index < End;  Index++)
    {
        if (!(Index & (4U - 1U)))
        {
            if (!(Index & (16U - 1U)))
                ErrorF("\n 0x%08X:", Index + vga256InfoRec.BIOSbase);
            ErrorF(" ");
        }
        if (Index < Start)
            ErrorF("  ");
        else
            ErrorF("%02X", BIOS[Index]);
    }

    ErrorF("\n");
}

/*
 * ATIPrintIndexedRegisters --
 *
 * Display a set of indexed byte-size registers when the server is invoked with
 * -verbose.
 */
static void
ATIPrintIndexedRegisters(const CARD16 Port,
                         const CARD8 Start_Index, const CARD8 End_Index,
                         const char * Name, const CARD16 GenS1)
{
    int Index;

    ErrorF("\n\n %s register values:", Name);
    for (Index = Start_Index;  Index < End_Index;  Index++)
    {
        if(!(Index & (4U - 1U)))
        {
            if (!(Index & (16U - 1U)))
                ErrorF("\n 0x%02X:", Index);
            ErrorF(" ");
        }
        if (Port == ATTRX)
            (void) inb(GenS1);          /* Reset flip-flop */
        ErrorF("%02X", GetReg(Port, Index));
    }

    if (Port == ATTRX)
    {
        (void) inb(GenS1);              /* Reset flip-flop */
        outb(ATTRX, 0x20U);             /* Turn on PAS bit */
    }
}

/*
 * ATIPrintRegisters --
 *
 * Display various registers when the server is invoked with -verbose.
 */
void
ATIPrintRegisters(void)
{
    int Index, Step, Limit;
    CARD8 misc = inb(R_GENMO);
    CARD8 dac_read, dac_mask;
    CARD8 crtc = ATI_CRTC_VGA;
    CARD32 IO_Value;

    ErrorF("\n Miscellaneous output register value: 0x%02X.", misc);

    if (misc & 0x01U)
    {
        ATIPrintIndexedRegisters(CRTX(ColourIOBase), 0, 64,
            "Colour CRT controller", 0);
        ATIPrintIndexedRegisters(ATTRX, 0, 32, "Attribute controller",
            GENS1(ColourIOBase));
    }
    else
    {
        ATIPrintIndexedRegisters(CRTX(MonochromeIOBase), 0, 64,
            "Monochrome CRT controller", 0);
        ATIPrintIndexedRegisters(ATTRX, 0, 32, "Attribute controller",
            GENS1(MonochromeIOBase));
    }

    ATIPrintIndexedRegisters(GRAX, 0, 16, "Graphics controller", 0);
    ATIPrintIndexedRegisters(SEQX, 0, 8, "Sequencer", 0);

    if (ATIChipHasVGAWonder)
        ATIPrintIndexedRegisters(ATIIOPortVGAWonder,
            xf86ProbeOnly ? 0x80U : ATIVGAOffset, 0xC0U,
            "ATI Extended VGA", 0);

    if (ATIChipHasSUBSYS_CNTL)
    {
        ErrorF("\n\n 8514/A registers:");
        for (Index = 0x02E8U;  Index <= 0x0FEE8;  Index += 0x0400U)
        {
            if (!((Index - 0x02E8U) & 0x0C00U))
                ErrorF("\n 0x%04X:", Index);
            ErrorF(" %04X", inw(Index));
        }

        if (ATIAdapter >= ATI_ADAPTER_MACH8)
        {
            ErrorF("\n\n Mach8/Mach32 registers:");
            for (Index = 0x2EEU;  Index <= 0x0FEEE;  Index += 0x0400U)
            {
                if (!((Index - 0x02EEU) & 0x0C00U))
                    ErrorF("\n 0x%04X:", Index);
                ErrorF(" %04X", inw(Index));
            }
        }
    }
    else if (ATIChip >= ATI_CHIP_88800GXC)
    {
        ErrorF("\n\n Mach64 %s registers:",
           (ATIIODecoding == SPARSE_IO) ? "sparse" : "block");
        Limit = ATIIOPort(IOPortTag(0x1FU, 0x3FU));
        Step = ATIIOPort(IOPortTag(0x01U, 0x01U)) - ATIIOBase;
        for (Index = ATIIOBase;  Index <= Limit;  Index += Step)
        {
            if (!(((Index - ATIIOBase) / Step) & 0x03U))
                ErrorF("\n 0x%04X:", Index);
            if (Index == ATIIOPortDAC_REGS)
                ErrorF(" %02X%02X%02X%02X",
                    inb(ATIIOPortDAC_REGS + 3), inb(ATIIOPortDAC_REGS + 2),
                    inb(ATIIOPortDAC_REGS + 1), inb(ATIIOPortDAC_REGS));
            else
            {
                IO_Value = inl(Index);

                if ((Index == ATIIOPortCRTC_GEN_CNTL) &
                    (IO_Value & CRTC_EXT_DISP_EN))
                    crtc = ATI_CRTC_MACH64;

                ErrorF(" %08X", IO_Value);
            }
        }

        if (ATIChip >= ATI_CHIP_264CT)
        {
            ErrorF("\n\n Mach64 PLL registers:");
            if (ATIChip == ATI_CHIP_264LT)
                Limit = 32;
            else
                Limit = 16;
            for (Index = 0;  Index < Limit;  Index++)
            {
                if (!(Index & 3))
                {
                    if (!(Index & 15))
                        ErrorF("\n 0x%02X:", Index);
                    ErrorF(" ");
                }
                ErrorF("%02X", ATIGetMach64PLLReg(Index));
            }
        }
    }

    ATISetDACIOPorts(crtc);

    ErrorF("\n\n"
           " DAC read index:  0x%02X\n"
           " DAC write index: 0x%02X\n"
           " DAC mask:        0x%02X\n\n"
           " DAC colour lookup table:",
           dac_read = inb(ATIIOPortDAC_READ),
           inb(ATIIOPortDAC_WRITE),
           dac_mask = inb(ATIIOPortDAC_MASK));

    outb(ATIIOPortDAC_MASK, 0xFFU);
    outb(ATIIOPortDAC_READ, 0x00U);

    for (Index = 0;  Index < 256;  Index++)
    {
        if (!(Index & 3))
            ErrorF("\n 0x%02X:", Index);
        ErrorF("  %02X", inb(ATIIOPortDAC_DATA));
        DACDelay;
        ErrorF(" %02X", inb(ATIIOPortDAC_DATA));
        DACDelay;
        ErrorF(" %02X", inb(ATIIOPortDAC_DATA));
        DACDelay;
    }

    outb(ATIIOPortDAC_MASK, dac_mask);
    outb(ATIIOPortDAC_READ, dac_read);

    ErrorF("\n\n");
}

/*
 * ATIPrintMode --
 *
 * This function prints out a mode's timing information on stderr.
 */
void
ATIPrintMode(DisplayModePtr mode)
{
    TokenTabPtr TokenEntry;
    int mode_flags = mode->Flags;

    ErrorF(" Dot clock:           %7.3fMHz\n"
           " Horizontal timings:  %4d %4d %4d %4d\n"
           " Vertical timings:    %4d %4d %4d %4d\n",
        (double)vga256InfoRec.clock[mode->Clock] / 1000.0,
        mode->HDisplay, mode->HSyncStart, mode->HSyncEnd, mode->HTotal,
        mode->VDisplay, mode->VSyncStart, mode->VSyncEnd, mode->VTotal);

    if (mode_flags & V_HSKEW)
    {
        mode_flags &= ~V_HSKEW;
        ErrorF(" Horizontal skew:     %4d\n", mode->HSkew);
    }

    ErrorF(" Flags:              ");

    for (TokenEntry = TokenTab;  TokenEntry->flag;  TokenEntry++)
        if (mode_flags & TokenEntry->flag)
        {
            ErrorF(" %s", TokenEntry->token);
            mode_flags &= ~TokenEntry->flag;
            if (!mode_flags)
                        break;
        }

    ErrorF("\n");
}

/*
 * ATIPrintMemoryType --
 *
 * This function is called by ATIProbe to print, on stderr, the amount and type
 * of video memory used by the adapter.
 */
void
ATIPrintMemoryType(const char *MemoryTypeName)
{
    ErrorF("%d kB of %s detected", ATIvideoRam, MemoryTypeName);
    if (ATIUsing1bppModes)
        ErrorF(" (using %d kB)", vga256InfoRec.videoRam / 4);
    else if (ATIvideoRam > vga256InfoRec.videoRam)
        ErrorF(" (using %d kB)", vga256InfoRec.videoRam);
    ErrorF(".\n");
}