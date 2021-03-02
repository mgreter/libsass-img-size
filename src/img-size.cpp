#include <cstring>
#include <iostream>
#include <stdio.h>
#include "sass.h"

// this function floats around the net since a long time
// hard to find the origin, but it should be public domain
// See also https://github.com/jmjatlanta/GraphicSize
bool get_image_size(const char *fn, size_t *x,size_t *y)
{
    FILE *f=fopen(fn,"rb");
    if (f==0) return false;
    fseek(f,0,SEEK_END);
    long len=ftell(f);
    fseek(f,0,SEEK_SET);
    if (len<24) {
        fclose(f);
        return false;
        }
  // Strategy:
  // reading GIF dimensions requires the first 10 bytes of the file
  // reading PNG dimensions requires the first 24 bytes of the file
  // reading JPEG dimensions requires scanning through jpeg chunks
  // In all formats, the file is at least 24 bytes big, so we'll read that always
  unsigned char buf[24];
  if (fread(buf,1,24,f) != 24)
    return false;

  // For JPEGs, we need to read the first 12 bytes of each chunk.
  // We'll read those 12 bytes at buf+2...buf+14, i.e. overwriting the existing buf.
  if (buf[0]==0xFF && buf[1]==0xD8 && buf[2]==0xFF && buf[3]==0xE0 && buf[6]=='J' && buf[7]=='F' && buf[8]=='I' && buf[9]=='F')
  { long pos=2;
    while (buf[2]==0xFF)
    { if (buf[3]==0xC0 || buf[3]==0xC1 || buf[3]==0xC2 || buf[3]==0xC3 || buf[3]==0xC9 || buf[3]==0xCA || buf[3]==0xCB) break;
      pos += 2+(buf[4]<<8)+buf[5];
      if (pos+12>len) break;
      fseek(f,pos,SEEK_SET);
      if (fread(buf+2,1,12,f) != 12)
        return false;
    }
  }

  fclose(f);

  // JPEG: (first two bytes of buf are first two bytes of the jpeg file; rest of buf is the DCT frame
  if (buf[0]==0xFF && buf[1]==0xD8 && buf[2]==0xFF)
  { *y = (buf[7]<<8) + buf[8];
    *x = (buf[9]<<8) + buf[10];
    //cout << *x << endl;
    return true;
  }

  // GIF: first three bytes say "GIF", next three give version number. Then dimensions
  if (buf[0]=='G' && buf[1]=='I' && buf[2]=='F')
  { *x = buf[6] + (buf[7]<<8);
    *y = buf[8] + (buf[9]<<8);
    return true;
  }

  // PNG: the first frame is by definition an IHDR frame, which gives dimensions
  if ( buf[0]==0x89 && buf[1]=='P' && buf[2]=='N' && buf[3]=='G' && buf[4]==0x0D && buf[5]==0x0A && buf[6]==0x1A && buf[7]==0x0A
    && buf[12]=='I' && buf[13]=='H' && buf[14]=='D' && buf[15]=='R')
  { *x = (buf[16]<<24) + (buf[17]<<16) + (buf[18]<<8) + (buf[19]<<0);
    *y = (buf[20]<<24) + (buf[21]<<16) + (buf[22]<<8) + (buf[23]<<0);
    return true;
  }

  return false;
}

// all functions need the same initial code
#define IMPLEMENT_IMG_SIZE(fn) \
  if (!sass_value_is_list(s_args)) { \
    return sass_make_error("Invalid arguments for " #fn); \
  } \
  if (sass_list_get_size(s_args) != 1) { \
    return sass_make_error("Exactly one arguments expected for " #fn); \
  } \
  struct SassValue* inp = sass_list_get_value(s_args, 0); \
  if (!sass_value_is_string(inp)) { \
    return sass_make_error("You must pass a string into " #fn); \
  } \
  std::string url(sass_string_get_value(inp)); \
  \
  if (url.empty()) { \
    return sass_make_error("Image filename is empty"); \
  } \
  \
  char* path = sass_compiler_find_file(url.c_str(), comp); \
  \
  if (path == 0 || *path == 0) { \
    std::string errmsg("Image url not found: "); \
    errmsg += url; \
    sass_free_c_string(path); \
    return sass_make_error(errmsg.c_str()); \
  } \
  \
  size_t w = 0, h = 0; \
  if (!get_image_size(path, &w, &h)) { \
    std::string errmsg("Error reading image size: "); \
    errmsg += url; \
    sass_free_c_string(path); \
    return sass_make_error(errmsg.c_str()); \
  } \
  sass_free_c_string(path); \


// return dimensions as a list with space delimiters
struct SassValue* fn_img_size(struct SassValue* s_args, struct SassCompiler* comp)
{
  IMPLEMENT_IMG_SIZE(img_size)
  struct SassValue* list = sass_make_list(SASS_SPACE, false);
  struct SassValue* width = sass_make_number(w, "px");
  struct SassValue* height = sass_make_number(h, "px");
  sass_list_push (list, width);
  sass_list_push (list, height);
  return list;
}

// return the width as a number with px as unit
struct SassValue* fn_img_width(struct SassValue  * s_args, struct SassCompiler* comp)
{
  IMPLEMENT_IMG_SIZE(img_size)
  return sass_make_number(w, "px");
}

// return the height as a number with px as unit
struct SassValue* fn_img_height(struct SassValue* s_args, struct SassCompiler* comp)
{
  IMPLEMENT_IMG_SIZE(img_size)
  return sass_make_number(h, "px");
}

// return version of libsass we are linked against
extern "C" const char* ADDCALL libsass_get_version() {
  return libsass_version();
}

// entry point for libsass to request custom functions from plugin
extern "C" void ADDCALL libsass_init_plugin(struct SassCompiler* compiler)
{

  // register image size functions with compiler
  sass_compiler_add_custom_function(compiler, sass_make_function("img-size($url)", fn_img_size, 0));
  sass_compiler_add_custom_function(compiler, sass_make_function("img-width($url)", fn_img_width, 0));
  sass_compiler_add_custom_function(compiler, sass_make_function("img-height($url)", fn_img_height, 0));

}
