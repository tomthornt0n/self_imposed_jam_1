#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int
WINAPI WinMain(HINSTANCE instance_handle,
               HINSTANCE prev_instance_handle,
               LPSTR command_line,
               int show_mode)
{
#define RESOURCES_DIR "..\\resources"
 
 char *directory_path = RESOURCES_DIR "\\*";
 
 WIN32_FIND_DATA ffd;
 HANDLE find;
 
 FILE *resources_f = fopen("resources.gen.c", "w");
 FILE *interface_f = fopen("../source/jam_game_resources.gen.c", "w");
 
 fprintf(interface_f,
         "typedef struct\n"
         "{\n"
         " size_t w;\n"
         " size_t h;\n"
         " Pixel *buffer;\n"
         "} RES_Texture;\n\n");
 
 fprintf(resources_f,
         "#include \"../source/jam_game_platform.c\"\n"
         "#include \"../source/jam_game_resources.gen.c\"\n\n");
 
 int rc = 1;
 for (find = FindFirstFileA(directory_path, &ffd);
      rc != 0;
      rc = FindNextFile(find, &ffd))
 {
  if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
  {
   fprintf(stderr, "ignoring directory \"%s\"\n", ffd.cFileName);
  }
  else
  {
   char path[MAX_PATH] = {0};
   snprintf(path, MAX_PATH - 1, "%s\\%s", RESOURCES_DIR, ffd.cFileName);
   
   char name[128] = {0};
   for (int i = 0;
        ffd.cFileName[i] != 0 && ffd.cFileName[i] != '.' && i < sizeof(name) - 1;
        i += 1)
   {
    char c = ffd.cFileName[i];
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        c == '_')
    {
     name[i] = c;
    }
   }
   
   char *extension = PathFindExtensionA(ffd.cFileName);
   if (0 == StrCmpCA(extension, ".png") ||
       0 == StrCmpCA(extension, ".psd") ||
       0 == StrCmpCA(extension, ".bmp") ||
       0 == StrCmpCA(extension, ".jpg") ||
       0 == StrCmpCA(extension, ".jpeg"))
   {
    int w, h, channels;
    unsigned char *pixels = stbi_load(path, &w, &h, &channels, 4);
    
    if (pixels)
    {
     fprintf(resources_f,
             "size_t RES_Internal_%s_w = %d;\n"
             "size_t RES_Internal_%s_h = %d;\n"
             "unsigned char RES_Internal_%s_buffer[] =\n{\n",
             name, w,
             name, h,
             name);
     
     for (int i = 0;
          i < w * h * 4;
          i += 4)
     {
      fprintf(resources_f, "0x%x, 0x%x, 0x%x, 0x%x, ",
              pixels[i + 2],
              pixels[i + 1],
              pixels[i + 0],
              pixels[i + 3]);
     }
     
     fprintf(resources_f,
             "\n};\n"
             "void\n"
             "RES_%sTextureGet(RES_Texture *result)\n"
             "{\n"
             " result->buffer = (Pixel *)RES_Internal_%s_buffer;\n"
             " result->w = RES_Internal_%s_w;\n"
             " result->h = RES_Internal_%s_h;\n"
             "}\n\n",
             name, name, name, name);
     
     fprintf(interface_f, "void RES_%sTextureGet(RES_Texture *result);\n", name);
    }
    else
    {
     fprintf(stderr, "could not load image \"%s\"\n", ffd.cFileName);
    }
   }
   else if (0 == StrCmpCA(extension, ".png"))
   {
    fprintf(stderr, "todo: pack audio \"%s\"\n", ffd.cFileName);
   }
   else
   {
    fprintf(stderr, "ignoring unrecognised resource type \"%s\"\n", ffd.cFileName);
   }
  }
 }
 
 fclose(resources_f);
 fclose(interface_f);
}