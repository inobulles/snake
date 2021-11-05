/* GIMP RGBA C-Source image dump (fat.c) */

static const struct {
  unsigned  	 width;
  unsigned  	 height;
  unsigned  	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  uint8_t 	 pixel_data[8 * 8 * 4 + 1];
} img_fat = {
  8, 8, 4,
  "\000\000\000\000\000\000\000\000t\271\377\177S\331\377\177S\331\377\177S\331\377\177\000\000"
  "\000\000\000\000\000\000\000\000\000\000t\271\377\177t\271\377\177\251\304\250\377\207\226\222"
  "\377S\331\377\177S\331\377\177\000\000\000\000\000\000\000\000t\271\377\177\271\246\250\377"
  "\251\324\250\377\251\266\250\377\251\266\250\377S\331\377\177\000\000\000\000\000\000"
  "\000\000t\271\377\177\251\304\250\377\207\226\222\377\251\304\250\377P\265\215"
  "\377S\331\377\177\000\000\000\000\000\000\000\000S\331\377\177\251\266\250\377\207\226\222"
  "\377\251\324\250\377\251\266\250\377a\245\215\377\000\000\000\000\000\000\000\000S\331\377"
  "\177\251\304\250\377\251\324\250\377\251\304\250\377\271\304\250\377t\271"
  "\377\177\000\000\000\000\000\000\000\000S\331\377\177S\331\377\177\251\266\250\377\207\226"
  "\222\377t\271\377\177t\271\377\177\000\000\000\000\000\000\000\000\000\000\000\000S\331\377\177S"
  "\331\377\177S\331\377\177t\271\377\177\000\000\000\000\000\000\000\000",
};
