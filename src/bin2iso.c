/*******************************************************************************
 * Convert .bin to .iso using CD-ROM Sector Library
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************************************
Utilities
*******************************************************************************/
/* Print error and exit */
void perror_exit(const char * msg)
{
    perror(msg);

    exit(1);
}

/* Print help and exit */
void help_exit(const char * arg)
{
    const char * name;

    if ((!(name = strrchr(arg, '/'))))
    {
        name = strrchr(arg, '\\');
    }

    name = name ? &name[1] : arg;

    printf("Usage: %s <input .bin> <output .iso>\n", name);

    exit(2);
}

/*******************************************************************************
main()
*******************************************************************************/
int main(int argc, const char ** argv)
{
    FILE *   in;
    FILE *   out;
    unsigned sector_num;

    sector_num = 0;

    /* Check args */
    if (argc != 3)
    {
        help_exit(argv[0]);
    }

    /* Open input file */
    if ((!(in = fopen(argv[1], "rb"))))
    {
        perror_exit("Error opening input file");
    }

    /* Check that the input file size is divisible by 2352 */
    {
        long in_size;

        if (fseek(in, 0, SEEK_END) == -1)
        {
            perror_exit("Error determining size of input file");
        }

        if ((in_size = ftell(in)) == -1)
        {
            perror_exit("Error determining size of input file");
        }

        if (in_size % 2352 != 0)
        {
            fprintf(stderr, "Error: Input file size not divisible by 2352\n");

            exit(1);
        }

        rewind(in);
    }

    /* Open output file */
    if ((!(out = fopen(argv[2], "wb"))))
    {
        perror_exit("Error opening output file");
    }

    /* Copy disc image data sector by sector */
    {
        sector_error error;
        char         sector[2352];
        const void * data;
        sector_mode  mode;

        while (fread(&sector[0], 2352, 1, in) == 1)
        {
            error = sector_analyze(&sector[0], &data, &mode);

            if (error)
            {
                if (error == SECTOR_ERROR_MODE_2_F1_AMBIGUOUS ||
                    error == SECTOR_ERROR_MODE_2_F2_AMBIGUOUS)
                {
                    printf("Warning: sector_analyze_sector(%u): %s\n",
                           sector_num,
                           sector_error_string(error));
                }
                else
                {
                    fprintf(stderr,
                            "Error: sector_analyze_sector(%u): %s\n",
                            sector_num,
                            sector_error_string(error));

                    exit(1);
                }
            }

            if (mode != SECTOR_MODE_1 && mode != SECTOR_MODE_2_FORM_1)
            {
                fprintf(stderr,
                        "Error: sector_analyze_sector(%u): "
                        "Non-data sector: %s\n",
                        sector_num,
                        sector_mode_string(mode));

                exit(1);
            }

            if (fwrite(data, 2048, 1, out) != 1)
            {
                perror_exit("Error writing output file");
            }

            sector_num++;
        }

        if (ferror(in))
        {
            perror_exit("Error reading input file");
        }
    }

    /* Cleanup */
    fclose(in);
    fclose(out);

    return 0;
}
