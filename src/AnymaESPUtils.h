#pragma ONCE
#include "LittleFS.h"
#include "AnymaEspSettings.h"

void file_list()
{
  File root, foundfile;

  root = MAIN_FILE_SYSTEM.open("/");
  if (root)
  {
    foundfile = root.openNextFile();
    while (foundfile)
    {
      log_v("File: %s Size %d", foundfile.name(), foundfile.size()); // force copy
      foundfile = root.openNextFile();
    }
    root.close();
    foundfile.close();
  }
}


String colorToHex(CRGB col)
{
  char buf[6];
  sprintf(buf, "%02X%02X%02X", col.r, col.g, col.b);
  return String(buf);
}

CRGB hexToColor(const char *s)
{
  uint32_t res = strtol(s, NULL, 16);
  return CRGB(res);
}