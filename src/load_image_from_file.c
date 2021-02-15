/* atlc - arbitrary transmission line calculator, for the analysis of
transmission lines are directional couplers. 

Copyright (C) 2002. Dr. David Kirkby, PhD (G8WRB).

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

Dr. David Kirkby, e-mail drkirkby at gmail.com 

*/

/* This function take a filename with the extension .bmp (eg coax.bmp) 
and will produce files such as coax.V.bmp, coax.E.bmp, coax.E.bin etc */

#include "config.h"

#include <sys/errno.h>

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

int
load_image_from_file(char *filename, size_t *size, int *width, int *height, unsigned char **image_data) {
  FILE *fp;
  int fnlen, offset;
  size_t i;

  fnlen = strlen(filename);
  if (fnlen < 5) {
    fprintf(stderr, "Error #5: Filename '%s' too short\n", filename);
    return 1;
  }

  if ((fp = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "Error #5: Unable to open file %s: %s\n", filename, strerror(errno));
    return 1;
  }

  if (!strcasecmp(filename + fnlen - 3, "bmp")) {
    image_type = IMG_BMP;
    read_bitmap_file_headers(fp, &offset, size, width, height);
    *image_data = ustring(0L,(long)*size);
    if (fseek(fp, offset, SEEK_SET) != 0) {
      fprintf(stderr,"Error #5: Unable to seek to BMP data at %d from %s\n", offset, filename);
      return 1;
    }
    /* For some unknown reason Microsoft's Visual C++ was unhappy to read
       the bitmap image using an fread() call. Instead, the following two
       stupid lines fixed that issue. This will only get compiled under
       Windoze, the more sensible fread call being used on other operating
       systems. */
#ifdef WINDOWS
    for(i = 0; i < (long)*size && (feof(image_data_fp) == 0); i++)
      *image_data[i] = (unsigned char)fgetc(fp);
#else
    i = fread(*image_data,  *size, 1, fp);
#endif
    if(ferror(fp) || i != 1)
    {
      fprintf(stderr,"Error #5. Unable to read all of the image data properly %ld %d from %s\n", i, ferror(fp), filename);
      exit_with_msg_and_exit_code("", 5);
    }
#ifdef HAVE_LIBPNG
  } else if (!strcasecmp(filename + fnlen - 3, "png")) {
    image_type = IMG_PNG;
    if (load_image_from_png(filename, size, width, height, image_data)) {
      fprintf(stderr, "Error #5: Unable to load image %s\n", filename);
      exit_with_msg_and_exit_code("",5);
    }
#endif
  } else {
    fprintf(stderr, "Error #5: Unknown file type %s\n", filename);
    return 1;
  }

  fclose(fp);
  return 0;
}
