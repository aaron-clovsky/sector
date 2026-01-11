/*******************************************************************************
 * CD-ROM Sector Library
 * Copyright (C) 2026 Aaron Clovsky
 * Based on CRDDAO
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/
#ifndef SECTOR_HEADER
#define SECTOR_HEADER

/*******************************************************************************
Macros
*******************************************************************************/
#ifndef _MSC_VER
    #define SECTOR_PACK(__declaration__)            \
        __declaration__ __attribute__((__packed__))
#else
    #define SECTOR_PACK(__declaration__)                            \
        __pragma(pack(push, 1)) __declaration__ __pragma(pack(pop))
#endif

#define SECTOR_PACK_DEF(__qualifier__, __type__, __declaration__) \
    SECTOR_PACK(__qualifier__ __type__ __declaration__);          \
    typedef __qualifier__ __type__ __type__

/*******************************************************************************
Headers
*******************************************************************************/
#include <stdint.h>

/*******************************************************************************
Types
*******************************************************************************/
typedef enum
{
    SECTOR_ERROR_NONE                = 0,
    SECTOR_ERROR_INVALID_SYNC        = 1,
    SECTOR_ERROR_INVALID_MODE        = 2,
    SECTOR_ERROR_MODE_2_F1_AMBIGUOUS = 3,
    SECTOR_ERROR_MODE_2_F2_AMBIGUOUS = 4
} sector_error;

typedef enum
{
    SECTOR_MODE_INVALID  = 0,
    SECTOR_MODE_0        = 1,
    SECTOR_MODE_1        = 2,
    SECTOR_MODE_2        = 3,
    SECTOR_MODE_2_FORM_1 = 4,
    SECTOR_MODE_2_FORM_2 = 5
} sector_mode;

SECTOR_PACK_DEF(struct, sector_header, {
    uint8_t sync[12];
    uint8_t offset[3];
    uint8_t mode_bits;
});

SECTOR_PACK_DEF(struct, sector_mode_0, {
    sector_header header;
    uint8_t       data[2336];
});

SECTOR_PACK_DEF(struct, sector_mode_1, {
    sector_header header;
    uint8_t       data[2048];
    uint32_t      edc;
    uint8_t       zero[8];
    uint8_t       ecc[276];
});

SECTOR_PACK_DEF(struct, sector_mode_2, {
    sector_header header;
    uint8_t       data[2336];
});

SECTOR_PACK_DEF(struct, sector_mode_2_form_1, {
    sector_header header;
    uint8_t       sub_header[8];
    uint8_t       data[2048];
    uint32_t      edc;
    uint8_t       ecc[276];
});

SECTOR_PACK_DEF(struct, sector_mode_2_form_2, {
    sector_header header;
    uint8_t       sub_header[8];
    uint8_t       data[2324];
    uint32_t      edc;
});

SECTOR_PACK_DEF(union, sector, {
    sector_mode_2_form_2 mode_0;
    sector_mode_2_form_2 mode_1;
    sector_mode_2_form_2 mode_2;
    sector_mode_2_form_2 mode_2_form_1;
    sector_mode_2_form_2 mode_2_form_2;
});

/*******************************************************************************
External functions
*******************************************************************************/
/* Analyze sector to determine mode and data location
   Note: data and/or mode may be passed as NULL */
sector_error sector_analyze(const void *  sector,
                            const void ** data,
                            sector_mode * mode);

/* Calculate sector EDC
   Returns zero if EDC does not exist for the given mode */
uint32_t sector_calc_edc(const void * sector, sector_mode mode);

/* Calculate sector ECC
   Notes:
   - ecc must point to at least 276 bytes
   - Writes nothing to *ecc if ECC cannot be calculated for the given mode*/
void sector_calc_ecc(const void * sector, sector_mode mode, uint8_t * ecc);

/* Stringify mode */
const char * sector_mode_string(sector_mode mode);

/* Stringify error */
const char * sector_error_string(sector_error error);

#endif
