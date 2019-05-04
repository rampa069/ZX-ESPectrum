// X origin to center an element with pixel_width
unsigned short scrAlignCenterX(unsigned short pixel_width) { return (SCR_W / 2) - (pixel_width / 2); }
// Y origin to center an element with pixel_height
unsigned short scrAlignCenterY(unsigned short pixel_height) { return (SCR_H / 2) - (pixel_height / 2); }

byte osdMaxRows() { return (OSD_H - (OSD_MARGIN * 2)) / OSD_FONT_H; }
byte osdMaxCols() { return (OSD_W - (OSD_MARGIN * 2)) / OSD_FONT_W; }
unsigned short osdInsideX() { return scrAlignCenterX(OSD_W) + OSD_MARGIN; }
unsigned short osdInsideY() { return scrAlignCenterY(OSD_H) + OSD_MARGIN; }
