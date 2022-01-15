
#ifdef _WIN32
# include <windows.h>
# include <shlwapi.h>
# define RE_InputDir "..\\resources"
#else
# include <sys/types.h>
# include <dirent.h>
# define RE_InputDir "../resources"
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


static char *
RE_ExtensionFromFilename(char *filename)
{
    char *last_dot = NULL;
    for (char *c = filename;
         '\0' != *c;
         c += 1)
    {
        if ('.' == *c)
        {
            last_dot = c;
        }
    }
    return last_dot;
}

typedef struct
{
    FILE *header;
    FILE *impl;
} RE_Files;

static RE_Files
RE_Init(void)
{
    RE_Files files =
    {
        .header = fopen("../source/jam_game_resources.gen.c", "w"),
        .impl = fopen("resources.gen.c", "w"),
    };
    
    fprintf(files.header,
            "#include <stddef.h>\n\n"
            "typedef struct\n"
            "{\n"
            " size_t w;\n"
            " size_t h;\n"
            " const Pixel *buffer;\n"
            "} RES_Texture;\n\n"
            
            "typedef struct\n"
            "{\n"
            " size_t size;\n"
            " unsigned char *wav;\n"
            "} RES_Audio;\n\n");
    
    fprintf(files.impl,
            "#include \"../source/jam_game_platform.c\"\n"
            "#include \"../source/jam_game_resources.gen.c\"\n\n");
    
    return files;
}

static void
RE_OutputResource(RE_Files output_files, char *resource_filename)
{
    int last_slash = 0;
    for(int i = 0;
        resource_filename[i] != '\0';
        i += 1)
    {
        if(resource_filename[i] == '/' ||
           resource_filename[i] == '\\')
        {
            last_slash = i;
        }
    }
    
    int j = 0;
    char name[128] = {0};
    for (int i = last_slash + 1;
         resource_filename[i] != 0 && resource_filename[i] != '.' && i < sizeof(name) - 1;
         i += 1)
    {
        char c = resource_filename[i];
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '_')
        {
            name[j] = c;
            j += 1;
        }
    }
    
    if('_' == name[0])
    {
        fprintf(stderr, "Skipping '%s'\n", name);
        return;
    }
    
    fprintf(stderr, "Outputting '%s' as '%s'\n", resource_filename, name);
    
    char *extension = RE_ExtensionFromFilename(resource_filename);
    if (0 == strcmp(extension, ".png") ||
        0 == strcmp(extension, ".psd") ||
        0 == strcmp(extension, ".bmp") ||
        0 == strcmp(extension, ".jpg") ||
        0 == strcmp(extension, ".jpeg"))
    {
        int w, h, channels;
        unsigned char *pixels = stbi_load(resource_filename, &w, &h, &channels, 4);
        
        if (pixels)
        {
            fprintf(output_files.impl,
                    "size_t RES_Internal_%sW = %d;\n"
                    "size_t RES_Internal_%sH = %d;\n"
                    "unsigned char RES_Internal_%sTextureBuffer[] =\n{\n",
                    name, w,
                    name, h,
                    name);
            
            for (int i = 0;
                 i < w * h * 4;
                 i += 4)
            {
                fprintf(output_files.impl, "0x%x, 0x%x, 0x%x, 0x%x, ",
                        pixels[i + 2],
                        pixels[i + 1],
                        pixels[i + 0],
                        pixels[i + 3]);
            }
            
            fprintf(output_files.impl,
                    "\n};\n"
                    "void\n"
                    "RES_%sTextureGet(RES_Texture *result)\n"
                    "{\n"
                    " result->buffer = (Pixel *)RES_Internal_%sTextureBuffer;\n"
                    " result->w = RES_Internal_%sW;\n"
                    " result->h = RES_Internal_%sH;\n"
                    "}\n\n",
                    name, name, name, name);
            
            fprintf(output_files.header, "void RES_%sTextureGet(RES_Texture *result);\n", name);
        }
        else
        {
            fprintf(stderr, "could not load image \"%s\": %s - %s\n", resource_filename, stbi_failure_reason(), strerror(errno));
        }
    }
    else if (0 == strcmp(extension, ".wav"))
    {
        FILE *audio_f = fopen(resource_filename, "rb");
        
        fprintf(output_files.impl, "unsigned char RES_Internal_%sWavBuffer[] =\n{\n", name);
        while (!feof(audio_f))
        {
            fprintf(output_files.impl, "0x%x, ", fgetc(audio_f));
        }
        
        fprintf(output_files.impl,
                "\n};\n"
                "void\n"
                "RES_%sAudioGet(RES_Audio *result)\n"
                "{\n"
                "result->size = sizeof(RES_Internal_%sWavBuffer);\n"
                "result->wav = RES_Internal_%sWavBuffer;\n"
                "}\n\n",
                name, name, name, name, name);
        
        fprintf(output_files.header, "void RES_%sAudioGet(RES_Audio *result);", name);
    }
    else
    {
        fprintf(stderr, "ignoring unrecognised resource type \"%s\"\n", resource_filename);
    }
}

#ifdef _WIN32
int
WINAPI WinMain(HINSTANCE instance_handle,
               HINSTANCE prev_instance_handle,
               LPSTR command_line,
               int show_mode)
{
    char *directory_resource_filename = RE_InputDir "\\*";
    
    RE_Files files = RE_Init();
    
    WIN32_FIND_DATA ffd;
    HANDLE find;
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
            snprintf(path, sizeof(path) - 1, "%s\\%s", RE_InputDir, ffd.cFileName);
            RE_OutputResource(files, path);
        }
    }
}

#else

int
main(int arguments_count, char **arguments)
{
    RE_Files files = RE_Init();
    
    DIR *d = opendir(RE_InputDir);
    if (NULL == d)
    {
        return -1;
    }
    
    struct dirent *dir;
    while (NULL != (dir = readdir(d)))
    {
        // ommit . and ..
        int should_ommit = strlen(dir->d_name) >= 1 && (dir->d_name[0] == '.' && (dir->d_name[1] == '\0' || (dir->d_name[1] == '.' && dir->d_name[2] == '\0')));
        if (!should_ommit)
        {
            char path_1[PATH_MAX] = {0};
            char path_2[PATH_MAX] = {0};
            snprintf(path_1, sizeof(path_1) - 1, "%s/%s", RE_InputDir, dir->d_name);
            realpath(path_1, path_2);
            RE_OutputResource(files, path_2);
        }
    }
    
    return 0;
}

#endif
