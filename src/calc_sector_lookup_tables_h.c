/*******************************************************************************
 * CD-ROM Sector Library lookup tables generator
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
#include <stdio.h>
#include <stdint.h>

/*******************************************************************************
Globals
*******************************************************************************/
uint32_t SECTOR_CRC_TABLE[256];
uint16_t SECTOR_COEFF_TABLE[43][256];

/*******************************************************************************
CRC Table calculation
*******************************************************************************/
/* Return width bits of value in reverse order */
uint32_t reverse_binary(uint32_t value, uint32_t width)
{
    uint32_t i, reverse;

    for (i = 0, reverse = 0; i < width; i++)
    {
        reverse <<= 1;
        if (value & 1) reverse |= 1;
        value >>= 1;
    }

    return reverse;
}

/* Compute CRC lookup table */
void calc_crc_table()
{
    uint32_t data;
    uint32_t i;

    for (i = 0; i < 256; i++)
    {
        uint32_t k;

        data = reverse_binary(i, 8) << 24;

        for (k = 0; k < 8; k++)
        {
            data = (data & 0x80000000) ? (data << 1) ^ 0x8001801B : data << 1;
        }

        data = reverse_binary(data, 32);

        SECTOR_CRC_TABLE[i] = data;
    }
}

/*******************************************************************************
Coefficient Table calculation
*******************************************************************************/
/* Calculate logarithm and inverse logarithm tables */
void calc_log_table(uint8_t (*log_table)[2][256])
{
    uint32_t b;
    uint32_t i;

    (*log_table)[0][255] = (uint8_t)0;
    (*log_table)[1][255] = (uint8_t)0;

    for (i = 0, b = 1; i < 255; i++)
    {
        (*log_table)[0][b] = (uint8_t)i;
        (*log_table)[1][i] = (uint8_t)b;

        b <<= 1;

        if (b & 0x100) b ^= 0x11d;
    }
}

/* Perform division in the GF(8) domain */
uint8_t gf8_div(uint8_t (*log_table)[2][256], uint8_t a, uint8_t b)
{
    int sum;

    if (a == 0) return 0;

    sum = (int)(*log_table)[0][a] - (int)(*log_table)[0][b];

    if (sum < 0) sum += 255;

    return (*log_table)[1][sum];
}

/* Compute the products of 0-255 with all Q coefficients
   Note: P parity coefficients are a subset of Q parity coefficients */
void calc_coeff_table()
{
    uint8_t  coeffs[2][45];
    uint8_t  log_table[2][256];
    uint32_t i;

    calc_log_table(&log_table);

    for (i = 0; i < 45; i++)
    {
        coeffs[0][i] =
            gf8_div(&log_table, log_table[1][44 - i], log_table[1][1]) ^ 1;
        coeffs[1][i] = log_table[1][44 - i] ^ 1;
    }

    for (i = 0; i < 45; i++)
    {
        coeffs[0][i] = gf8_div(&log_table, coeffs[0][i], coeffs[0][44]);
        coeffs[1][i] = gf8_div(&log_table, coeffs[1][i], coeffs[1][43]);
    }

    for (i = 0; i < 43; i++)
    {
        uint16_t c;
        uint32_t k;

        SECTOR_COEFF_TABLE[i][0] = 0;

        for (k = 1; k < 256; k++)
        {
            c = (uint16_t)log_table[0][k] +
                (uint16_t)log_table[0][coeffs[0][i]];
            if (c >= 255) c -= 255;
            SECTOR_COEFF_TABLE[i][k] = log_table[1][c];

            c = (uint16_t)log_table[0][k] +
                (uint16_t)log_table[0][coeffs[1][i]];
            if (c >= 255) c -= 255;
            SECTOR_COEFF_TABLE[i][k] |= (uint16_t)log_table[1][c] << 8;
        }
    }
}

/*******************************************************************************
Produce header file
*******************************************************************************/
int main()
{
    unsigned i;
    unsigned k;

    calc_crc_table();
    calc_coeff_table();

    puts("/***************************************"
         "****************************************\n"
         " * CD-ROM Sector Library EDC/ECC Lookup Tables\n"
         " * Copyright (C) 2026 Aaron Clovsky\n"
         " * Based on CRDDAO\n"
         " ***************************************"
         "***************************************/\n"
         "#ifndef SECTOR_LOOKUP_TABLES_HEADER\n"
         "#define SECTOR_LOOKUP_TABLES_HEADER\n");
    puts("/***************************************"
         "****************************************\n"
         "Headers\n"
         "****************************************"
         "***************************************/\n"
         "#include <stdint.h>\n\n"
         "/***************************************"
         "****************************************\n"
         "Constants\n"
         "****************************************"
         "***************************************/\n"
         "static const uint32_t SECTOR_CRC_TABLE[256] = {");

    for (i = 0; i < 64; i++)
    {
        printf("    0x%08X, 0x%08X, 0x%08X, 0x%08X%s\n",
               SECTOR_CRC_TABLE[i * 4 + 0],
               SECTOR_CRC_TABLE[i * 4 + 1],
               SECTOR_CRC_TABLE[i * 4 + 2],
               SECTOR_CRC_TABLE[i * 4 + 3],
               (i != 63) ? "," : "");
    }

    puts("};\n\nstatic const uint16_t SECTOR_COEFF_TABLE[43][256] = {");

    for (i = 0; i < 43; i++)
    {
        puts("    {");

        for (k = 0; k < 32; k++)
        {
            printf("        0x%04X, 0x%04X, 0x%04X, 0x%04X, "
                   "0x%04X, 0x%04X, 0x%04X, 0x%04X%s\n",
                   SECTOR_COEFF_TABLE[i][k * 4 + 0],
                   SECTOR_COEFF_TABLE[i][k * 4 + 1],
                   SECTOR_COEFF_TABLE[i][k * 4 + 2],
                   SECTOR_COEFF_TABLE[i][k * 4 + 3],
                   SECTOR_COEFF_TABLE[i][k * 4 + 4],
                   SECTOR_COEFF_TABLE[i][k * 4 + 5],
                   SECTOR_COEFF_TABLE[i][k * 4 + 6],
                   SECTOR_COEFF_TABLE[i][k * 4 + 7],
                   (k != 31) ? "," : "");
        }

        puts((i != 42) ? "    }," : "    }");
    }

    puts("};\n\n#endif");

    return 0;
}
