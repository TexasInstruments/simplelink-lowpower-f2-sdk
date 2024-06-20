#include <stdio.h>
#include <stdint.h>
#include "slate_test.h"
#include "bootutil/image.h"


void slate_test_print(void)
{
#if defined(SLATE_TEST)
    uint8_t *exe_footprint = (uint8_t*)0x20000000;
    extern int MCUBOOT_HDR_BASE;
    struct image_header *mcubootHdr_slate = (struct image_header *)&MCUBOOT_HDR_BASE;
#if defined(DUAL_SLOT)
    struct image_header *slot2Hdr = (struct image_header *)SLOT_2_HDR_BASE;
    sprintf((char*)exe_footprint, "%s %d.%d.%d.%d %s %d.%d.%d.%ld", APP_BANNER,
                                    mcubootHdr_slate->ih_ver.iv_major,
                                    mcubootHdr_slate->ih_ver.iv_minor,
                                    mcubootHdr_slate->ih_ver.iv_revision,
                                    mcubootHdr_slate->ih_ver.iv_build_num,
                                    "SLOT_2",
                                    slot2Hdr->ih_ver.iv_major,
                                    slot2Hdr->ih_ver.iv_minor,
                                    slot2Hdr->ih_ver.iv_revision,
                                    slot2Hdr->ih_ver.iv_build_num);
#else
    sprintf((char*)exe_footprint, "%s %d.%d.%d.%ld", APP_BANNER,
                                    mcubootHdr_slate->ih_ver.iv_major,
                                    mcubootHdr_slate->ih_ver.iv_minor,
                                    mcubootHdr_slate->ih_ver.iv_revision,
                                    mcubootHdr_slate->ih_ver.iv_build_num);
#endif // DUAL_SLOT 
#endif // SLATE_TEST
}

