const uint32_t NUM_PIXELS = WIDTH * HEIGHT;

// 
// These are parameters for our pixel permutation done in O(1) space. Followed from https://stackoverflow.com/a/10054982
//
// Since our framebuffer takes ~93% of RAM, we can't do a naive array shuffle because it won't fit in RAM.
// So instead we use a property: G^k mod P for k = 1..(P-1) is a permutation of 1..(P-1)
// when P is prime and G is a primitive root of P
//

// We lucked out with our framebuffer size. WIDTH*HEIGHT + 1 was prime!
const uint32_t P = 600 * 700 + 1;

// This primitive root was computed separately and chosen. See primitive_root_finder.py
const uint32_t PRIM_ROOT = 2050;

/// This code runs on Core 1 of the RP2040, and handles the actual drawing logic
///
/// We can't actually use any Arduino SDK code here, only functions from the Pico SDK.
/// In our case we just perform pure compute and RAM accesses
void core1_entry() {
  // Keep track of the current frame being drawn
  uint32_t current_frame = 0;

  // Keep computing on repeat
  while (true) {
    // Prepare the random permutation of pixel coordinates
    uint32_t rand_index = 1;
    for (uint32_t k = 1; k <= (P - 1); k += 1) {
      // Compute G^k mod P
      rand_index *= PRIM_ROOT;
      rand_index %= P;

      // If we index out of the framebuffer, ignore it
      if (rand_index >= NUM_PIXELS) {
        continue;
      }

      // Convert the random index to row/col coords
      uint16_t row = rand_index / WIDTH;
      uint16_t col = rand_index % WIDTH;

      // Check if Core0 wants us to wait
      while (hold_core1) {
        // Keep this core idle for a little while
        sleep_ms(1);
      }

      // Grab the current colour and invert it so we can highlight it on the scan-out
      uint8_t color = get_color_at(row, col);
      set_color_at(row, col, color ^ 0b1111);

      // Then call the draw kernel
      // Note: row,col -> y, x
      color = draw_kernel(col, row, current_frame);

      // And write the actual colour
      set_color_at(row, col, color);
    }

    // Once we're done, move to the next frame
    current_frame += 1;
  }
}

// This function describes what colour to give to each pixel.
//
// Takes in the x/y coordinate and current frame number, and returns
// the 0-15 gray level colour for that pixel
uint8_t draw_kernel(uint16_t x, uint16_t y, uint32_t frame) {
  // This function toggles between the Mandelbrot set and the Newton
  // fractal on each frame, while slowly rotating the complex plane
  // on each frame

  // Normalize to 0-2 ish, keeping square dimensions
  const uint16_t scale = min(WIDTH, HEIGHT) / 4;

  double u = ((double) y) / scale;
  double v = ((double) x) / scale;

  // And shift it to fit -1:1 within the screen
  u -= 2.5;
  v -= 2;

  // Prepare the complex coordinates for this pixel
  double rot_angle = 2.0 * PI * frame / 120;
  _Complex uv = (u + v * 1.0I) * (cos(rot_angle) + sin(rot_angle) * 1.0I);

  // Toggle between Newton and Mandelbrot each frame, for flair
  if (frame % 2 == 0) {
    return mandelbrot(uv, frame);
  } else {
    return newton(uv, frame);
  }
}

uint8_t mandelbrot(_Complex uv, uint32_t frame) {
  _Complex z = 0;
  _Complex c = uv;

  uint8_t color;
  for (color = 15; color >= 1; color -= 1) {
    z = (z * z) + c;

    double dist = complex_arg_squared(z);

    if (dist > 4.0) {
      break;
    }
  }

  return color;
}

uint8_t newton(_Complex uv, uint32_t frame) {
  _Complex z = uv;

  uint8_t color;
  for (color = 0; color < 16; color += 1) {
    _Complex new_z = newt_func(z) / newt_deriv(z);

    double dist = complex_arg_squared(new_z - z);
    if (dist < 0.1) {
      break;
    }

    z = new_z;
  }

  return color;
}

_Complex newt_func(_Complex z) {
  return z * z * z - 1;
}

_Complex newt_deriv(_Complex z) {
  return 3 * z * z;
}

// === Complex utils

double complex_arg_squared(_Complex z) {
  return (__real__ z) * (__real__ z) + (__imag__ z) * (__imag__ z);
}
