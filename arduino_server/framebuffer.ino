/// Checks if the provided coordinate is within bounds of the framebuffer
inline bool out_of_fb_bounds(size_t row, size_t col) {
  return (row < 0 || row > HEIGHT) || (col < 0 || col > WIDTH);
}

/// Returns the 0-15 colour at the given framebuffer coordinate
uint8_t get_color_at(size_t row, size_t col) {
  // Out of bounds is black
  if (out_of_fb_bounds(row, col)) {
    return 0;
  }

  // Take the byte containing the pixel
  uint8_t out_byte = framebuffer[row][col / 2];

  // Extract the required nibble
  bool hi_nibble = (col % 2) == 0;
  if (hi_nibble) {
    out_byte >>= 4;
  } else {
    out_byte &= 0b1111;
  }

  return out_byte;
}

/// Sets a 0-15 colour at the given framebuffer coordinate
void set_color_at(size_t row, size_t col, uint8_t color) {
  // Out of bounds is ignored
  if (out_of_fb_bounds(row, col)) {
    return;
  }

  // Take the existing byte
  uint8_t out_byte = framebuffer[row][col / 2];

  // Set the correct nibble
  bool hi_nibble = (col % 2) == 0;
  if (hi_nibble) {
    out_byte = out_byte & 0b00001111 | (color << 4);
  } else {
    out_byte = out_byte & 0b11110000 | (color & 0b1111);
  }

  // Set it back
  framebuffer[row][col / 2] = out_byte;
}