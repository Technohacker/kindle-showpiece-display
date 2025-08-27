#pragma once

// File header type for BMP
struct __attribute__((packed)) bmp_file_header_t {
  uint8_t   signature[2];   // The file signature "BM"
  uint32_t  file_size;      // File size in bytes
  uint32_t  reserved = 0;   // Not used
  uint32_t  data_offset;    // Offset to image data in bytes from beginning of file
};

// File info type for BMP
struct __attribute__((packed)) bmp_info_header_t {
  uint32_t  dib_header_size = sizeof(bmp_info_header_t);   // Size of this header
  int32_t   width_px = WIDTH;           // Width of the image
  int32_t   height_px = HEIGHT;         // Height of image
  uint16_t  num_planes = 1;             // Number of color planes
  uint16_t  bits_per_pixel = 4;         // Bits per pixel
  uint32_t  compression = 0;            // Compression type (0 = uncompressed)
  uint32_t  image_size_bytes = 0;       // Image size in bytes (safe to set to 0 for uncompressed images)
  int32_t   x_resolution_ppm = WIDTH;   // Pixels per meter (probably safe to ignore)
  int32_t   y_resolution_ppm = HEIGHT;  // Pixels per meter (probably safe to ignore)
  uint32_t  num_colors = 16;            // Number of colors in the image
  uint32_t  important_colors = 0;       // Important colors (0 = all)
};

// RGB colour for BMP palettes
struct __attribute__((packed)) color_t {
  uint8_t r, g, b;
  uint8_t reserved = 0;
};

// The actual BMP Header used for each response
static const struct __attribute__((packed)) {
  bmp_file_header_t file_header;
  bmp_info_header_t info_header;
  color_t color_palette[16];
} FINAL_BMP_HEADER PROGMEM = {
  .file_header = {
    .signature = {'B', 'M'},
    .file_size = sizeof(FINAL_BMP_HEADER) + sizeof(framebuffer_t),
    .reserved = 0,
    .data_offset = sizeof(FINAL_BMP_HEADER),
  },

  .info_header = {},

  // Grayscale palette used by the Kindle
  .color_palette = {
    { .r =  0x00, .g =  0x00, .b =  0x00 }, // #000000
    { .r =  0x11, .g =  0x11, .b =  0x11 }, // #111111
    { .r =  0x22, .g =  0x22, .b =  0x22 }, // #222222
    { .r =  0x33, .g =  0x33, .b =  0x33 }, // #333333
    { .r =  0x44, .g =  0x44, .b =  0x44 }, // #444444
    { .r =  0x55, .g =  0x55, .b =  0x55 }, // #555555
    { .r =  0x66, .g =  0x66, .b =  0x66 }, // #666666
    { .r =  0x77, .g =  0x77, .b =  0x77 }, // #777777
    { .r =  0x88, .g =  0x88, .b =  0x88 }, // #888888
    { .r =  0x99, .g =  0x99, .b =  0x99 }, // #999999
    { .r =  0xaa, .g =  0xaa, .b =  0xaa }, // #aaaaaa
    { .r =  0xbb, .g =  0xbb, .b =  0xbb }, // #bbbbbb
    { .r =  0xcc, .g =  0xcc, .b =  0xcc }, // #cccccc
    { .r =  0xdd, .g =  0xdd, .b =  0xdd }, // #dddddd
    { .r =  0xee, .g =  0xee, .b =  0xee }, // #eeeeee
    { .r =  0xff, .g =  0xff, .b =  0xff }, // #ffffff
  }
};