/* atlc - arbitrary transmission line calculator, for the analysis of
transmission lines are directional couplers.

Copyright (C) 2021 Daniel O'Connor <darius@dons.net.au>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either package_version 2
of the License, or (at your option) any later package_version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
USA.

*/

#include "config.h"

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "definitions.h"
#include "exit_codes.h"

extern unsigned char *bmp_buff;

int write_data_to_image(FILE *fp, int width, int height, size_t size, unsigned char *image_data) {
  if (image_type == IMG_BMP) {
    if (fwrite(bmp_buff, 0x36, 1, fp) != 1) {
      fprintf(stderr, "Unable to write BMP header\n");
      return 1;
    }
    if (fwrite((void *) image_data, size, 1, fp) != 1) {
      fprintf(stderr, "Unable to write BMP data\n");
      return 1;
    }
#ifdef HAVE_LIBPNG
  } else {
    return write_data_to_png(fp, width, height, image_data);
#endif
  }
  return 0;
}
