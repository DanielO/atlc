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

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_SETJMP_H
#include <setjmp.h>
#endif

#include "definitions.h"
#include "exit_codes.h"

#include <png.h>
int
load_image_from_png(char *filename, size_t *size, int *width, int *height, unsigned char **image_data) {
  FILE		*fp;
  int		y;
  uint8_t	header[8];
  png_structp	png_ptr;
  png_infop   	info_ptr;
  png_infop	end_info;
  png_bytep	*row_pointers;
  unsigned char *ptr;

  if ((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL) {
    fprintf(stderr, "Unable to init PNG library\n");
    return 1;
  }

  if ((info_ptr = png_create_info_struct(png_ptr)) == NULL) {
    fprintf(stderr, "Unable to create PNG info struct\n");
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    return 1;
  }

  if ((end_info = png_create_info_struct(png_ptr)) == NULL) {
    fprintf(stderr, "Unable to create PNG end info struct\n");
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    return 1;
  }

  if ((fp = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "Unable to open '%s'\n", filename);
    return 1;
  }

  fread(header, 1, sizeof(header), fp);
  if (png_sig_cmp(header, 0, sizeof(header))) {
    fprintf(stderr, "'%s' does not appear to be a PNG file\n", filename);
    return 1;
  }

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, sizeof(header));

  png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_PACKING | PNG_TRANSFORM_STRIP_ALPHA | PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_BGR, NULL);

  *width = png_get_image_width(png_ptr, info_ptr);
  *height = png_get_image_height(png_ptr, info_ptr);
  *size = *width * *height * 3;
  *image_data = ustring(0L, (long)*size);
  ptr = *image_data;

  if ((row_pointers = png_get_rows(png_ptr, info_ptr)) == NULL) {
    fprintf(stderr, "Failed to read PNG into memory\n");
    return 1;
  }

  for (y = 0; y < *height; y++) {
    /* BMP is stored bottom up, invert the PNG so we get the same result */
    bcopy(row_pointers[*height - y - 1], ptr, *width * 3);
    ptr += *width * 3;
  }

  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  fclose(fp);

  return 0;
}

int
write_data_to_png(FILE *fp, int width, int height, unsigned char *image_data) {
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;
  int i;

  if ((row_pointers = png_malloc(png_ptr, height * (sizeof (png_bytep)))) == NULL) {
    fprintf(stderr, "Unable to allocate PNG row pointers\n");
    return 1;
  }

  if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
     NULL, NULL, NULL)) == NULL) {
    png_free(png_ptr, row_pointers);
    fprintf(stderr, "Unable to allocate PNG write struct\n");
    return 1;
  }

  if ((info_ptr = png_create_info_struct(png_ptr)) == NULL) {
    fprintf(stderr, "Unable to allocate PNG info struct\n");
    png_free(png_ptr, row_pointers);
    png_destroy_write_struct(&png_ptr, NULL);
    return 1;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_free(png_ptr, row_pointers);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fprintf(stderr, "PNG error during write\n");
    return 1;
  }

  png_init_io(png_ptr, fp);
  png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
   PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  for (i = 0; i < height; i++)
      row_pointers[height - i - 1] = image_data + i * width * 3;

  png_set_rows(png_ptr, info_ptr, row_pointers);

  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

  png_free(png_ptr, row_pointers);
  png_destroy_write_struct(&png_ptr, NULL);

  return 0;
}
