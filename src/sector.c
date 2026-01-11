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

/*******************************************************************************
Headers
*******************************************************************************/
#include <sector.h>
#include <sector_lookup_tables.h>
#include <string.h>

/*******************************************************************************
Internal functions
*******************************************************************************/
/* Calculate the P parities for the sector */
static void calc_p_parity(const uint8_t * sector, uint8_t * p_parity)
{
    unsigned i;

    for (i = 0; i < 43; i++)
    {
        uint16_t        lsb;
        uint16_t        msb;
        const uint8_t * p;
        unsigned        dbl_i;
        unsigned        k;

        lsb   = 0;
        msb   = 0;
        dbl_i = i << 1;
        p     = &sector[dbl_i + 12];

        for (k = 19; k < 43; k++)
        {
            lsb ^= SECTOR_COEFF_TABLE[k][p[0]];
            msb ^= SECTOR_COEFF_TABLE[k][p[1]];

            p += 86;
        }

        p_parity[dbl_i + 0]  = (uint8_t)(lsb >> 8);
        p_parity[dbl_i + 1]  = (uint8_t)(msb >> 8);
        p_parity[dbl_i + 86] = (uint8_t)(lsb);
        p_parity[dbl_i + 87] = (uint8_t)(msb);
    }
}

/* Calculate the Q parities for the sector */
static void calc_q_parity(const uint8_t * sector, uint8_t * q_parity)
{
    unsigned i;

    for (i = 0; i < 26; i++)
    {
        uint16_t        lsb;
        uint16_t        msb;
        const uint8_t * q;
        unsigned        dbl_i;
        unsigned        k;

        lsb   = 0;
        msb   = 0;
        dbl_i = i << 1;
        q     = &sector[(i * 86) + 12];

        for (k = 0; k < 43; k++)
        {
            lsb ^= SECTOR_COEFF_TABLE[k][q[0]];
            msb ^= SECTOR_COEFF_TABLE[k][q[1]];

            q += (q >= &sector[2160]) ? -2148 : 88;
        }

        q_parity[dbl_i + 0]  = (uint8_t)(lsb >> 8);
        q_parity[dbl_i + 1]  = (uint8_t)(msb >> 8);
        q_parity[dbl_i + 52] = (uint8_t)(lsb);
        q_parity[dbl_i + 53] = (uint8_t)(msb);
    }
}

/*******************************************************************************
External functions
*******************************************************************************/
/*
    Analyze sector to determine mode and data location
*/
sector_error sector_analyze(const void *  sector,
                            const void ** data,
                            sector_mode * mode)
{
    const uint8_t         SYNC_DATA[] = { 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
                                          0xff, 0xff, 0xff, 0xff, 0xff, 0x00 };
    const sector_header * header;
    unsigned              mode_bits;

    header = (const sector_header *)sector;

    if (memcmp(&header->sync[0], &SYNC_DATA[0], sizeof(SYNC_DATA)) != 0)
    {
        return SECTOR_ERROR_INVALID_SYNC;
    }

    mode_bits = header->mode_bits & 3;

    if ((header->mode_bits & ~3) != 0 || mode_bits == 3)
    {
        return SECTOR_ERROR_INVALID_MODE;
    }

    if (mode_bits == 0)
    {
        if (mode) *mode = SECTOR_MODE_0;

        if (data) *data = &((const sector_mode_0 *)sector)->data[0];

        return SECTOR_ERROR_NONE;
    }
    else if (mode_bits == 1)
    {
        if (mode) *mode = SECTOR_MODE_1;

        if (data) *data = &((const sector_mode_1 *)sector)->data[0];

        return SECTOR_ERROR_NONE;
    }
    else
    {
        const sector_mode_2_form_1 * form_1;
        const sector_mode_2_form_2 * form_2;
        unsigned                     form_bit;
        uint32_t                     edc;

        form_1 = (const sector_mode_2_form_1 *)sector;
        form_2 = (const sector_mode_2_form_2 *)sector;

        /* If subheader data does not repeat then sector is vanilla Mode 2 */
        if (memcmp(&form_1->sub_header[0], &form_1->sub_header[4], 4) != 0)
        {
            if (mode) *mode = SECTOR_MODE_2;

            if (data) *data = &((const sector_mode_2 *)sector)->data[0];

            return SECTOR_ERROR_NONE;
        }

        form_bit = form_1->sub_header[2] & 0x20;

        /* Check EDC to confirm subheader data */
        if (!form_bit)
        {
            edc = sector_calc_edc(sector, SECTOR_MODE_2_FORM_1);

            if (mode) *mode = SECTOR_MODE_2_FORM_1;

            if (data) *data = &form_1->data[0];

            if (edc == form_1->edc)
            {
                return SECTOR_ERROR_NONE;
            }

            return SECTOR_ERROR_MODE_2_F1_AMBIGUOUS;
        }
        else
        {
            edc = sector_calc_edc(sector, SECTOR_MODE_2_FORM_2);

            if (mode) *mode = SECTOR_MODE_2_FORM_2;

            if (data) *data = &form_2->data[0];

            if (edc == form_2->edc)
            {
                return SECTOR_ERROR_NONE;
            }

            return SECTOR_ERROR_MODE_2_F2_AMBIGUOUS;
        }
    }
}

/*
    Calculate sector EDC
*/
uint32_t sector_calc_edc(const void * sector, sector_mode mode)
{
    const uint8_t * data;
    unsigned        len;
    uint32_t        crc;

    if (mode == SECTOR_MODE_1)
    {
        data = (const uint8_t *)sector;
        len  = 16 + 2048; /* Sector header + 2048 bytes data */
    }
    else if (mode == SECTOR_MODE_2_FORM_1)
    {
        data = (const uint8_t *)sector + sizeof(sector_header);
        len  = 8 + 2048; /* XA subheader + 2048 bytes data */
    }
    else if (mode == SECTOR_MODE_2_FORM_2)
    {
        data = (const uint8_t *)sector + sizeof(sector_header);
        len  = 8 + 2324; /* XA subheader + 2324 bytes data */
    }
    else
    {
        return 0;
    }

    crc = 0;

    while (len--)
    {
        crc = SECTOR_CRC_TABLE[(crc ^ *data++) & 0xff] ^ (crc >> 8);
    }

    return crc;
}

/*
    Calculate sector ECC
*/
void sector_calc_ecc(const void * sector, sector_mode mode, uint8_t * ecc)
{
    uint8_t copy[2352];

    if (mode == SECTOR_MODE_1)
    {
        memcpy(&copy[0], sector, 2352);
        *(uint32_t *)&copy[2068] = 0;
        *(uint32_t *)&copy[2072] = 0;
        calc_p_parity(&copy[0], &ecc[0]);
        calc_q_parity(&copy[0], &ecc[172]);
        memcpy(ecc, &copy[2076], 276);
    }
    else if (mode == SECTOR_MODE_2_FORM_1)
    {
        memcpy(&copy[0], sector, 2352);
        *(uint32_t *)&copy[12] = 0;
        calc_p_parity(&copy[0], &copy[2076]);
        calc_q_parity(&copy[0], &copy[2248]);
        memcpy(ecc, &copy[2076], 276);
    }
}

/*
    Stringify mode
*/
const char * sector_mode_string(sector_mode mode)
{
    /* clang-format off */
    switch (mode)
    {
        case SECTOR_MODE_INVALID:
            return "Invalid mode";
        case SECTOR_MODE_0:
            return "Mode 0";
        case SECTOR_MODE_1:
            return "Mode 1";
        case SECTOR_MODE_2:
            return "Mode 2";
        case SECTOR_MODE_2_FORM_1:
            return "Mode 2 Form 1";
        case SECTOR_MODE_2_FORM_2:
            return "Mode 2 Form 2";
        default:
            return "Unknown error";
    }
    /* clang-format on */
}

/*
    Stringify error
*/
const char * sector_error_string(sector_error error)
{
    /* clang-format off */
    switch (error)
    {
        case SECTOR_ERROR_NONE:
            return "No error";
        case SECTOR_ERROR_INVALID_SYNC:
            return "Invalid sector synchronization data";
        case SECTOR_ERROR_INVALID_MODE:
            return "Invalid sector mode value";
        case SECTOR_ERROR_MODE_2_F1_AMBIGUOUS:
            return "Sector is either Mode 2 or Mode 2 Form 1 with corrupt EDC";
        case SECTOR_ERROR_MODE_2_F2_AMBIGUOUS:
            return "Sector is either Mode 2 or Mode 2 Form 2 with corrupt EDC";
        default:
            return "Unknown error";
    }
    /* clang-format on */
}
