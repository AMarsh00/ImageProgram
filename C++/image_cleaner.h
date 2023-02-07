#pragma once

// I'm using some things that are technically depreciated, so this blocks warnings on that
#define _CRT_SECURE_NO_WARNINGS

// stb_image.h kept raising warning 4244, so I disabled it
#pragma warning (disable : 4244)

// Include necessary libraries
#include <Windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <thread>

#include "stb_image.h"
#include "quick_select.h"

// Ffmpeg is a C library, so we have to specify that it's C, not C++
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libswscale/swscale.h>
}

// Link with ffmpeg libraries. These have to be in the same folder as the .exe (currently under x64/Release)
#pragma comment(lib, "D:\\ImageProgram\\ffmpeg-master-latest-win64-gpl-shared\\lib\\avcodec.lib")
#pragma comment(lib, "D:\\ImageProgram\\ffmpeg-master-latest-win64-gpl-shared\\lib\\avformat.lib")
#pragma comment(lib, "D:\\ImageProgram\\ffmpeg-master-latest-win64-gpl-shared\\lib\\avdevice.lib")
#pragma comment(lib, "D:\\ImageProgram\\ffmpeg-master-latest-win64-gpl-shared\\lib\\avfilter.lib")
#pragma comment(lib, "D:\\ImageProgram\\ffmpeg-master-latest-win64-gpl-shared\\lib\\avutil.lib")
#pragma comment(lib, "D:\\ImageProgram\\ffmpeg-master-latest-win64-gpl-shared\\lib\\swscale.lib")

// If you compile this on a different computer, you will have to download ffmpeg and set it up (not fun)

#define BASIC_FAST_MEDIAN_RANGE 5 // Fast median will return average if max is within FAST_MEDIAN_RANGE of the min
#define BASIC_FAST_MODE_RANGE 5 // Fast mode will return average if max is within FAST_MODE_RANGE of the min

// Comment back in #define CHECK_BAD_ALLOC to break if malloc or calloc fails (debug feature)
//#define CHECK_BAD_ALLOC

// Comment back in #define NO_MEAN, #define NO_MEDIAN, or #define NO_MODE to not do those
//#define NO_MEAN
//#define NO_MEDIAN
//#define NO_MODE

// Comment back in #define PRINT_PIXELS_PER_SECOND, #define PRINT_NUM_MEANS, #define PRINT_NUM_MEDIANS, #define PRINT_NUM_MODES to print those things
//#define PRINT_PIXELS_PER_SECOND
//#define PRINT_NUM_MEANS
//#define PRINT_NUM_MEDIANS
//#define PRINT_NUM_MODES

// Comment back in #define PRETTY_PRINT to pretty print outputs (only relevant if the above output macros are commented in, and will take a little overhead)
//#define PRETTY_PRINT

// Comment back in #define BASIC_FAST_MEDIAN_MODE to use a faster, but slightly less accurate, median and mode function
//#define BASIC_FAST_MEDIAN_MODE

// Comment back in #define VERY_FAST_MEDIAN_MODE to use a much faster, but much more innacurate, median and mode function
// Note that BASIC_FAST_MEDIAN_MODE will override this if you have both commented in
#define VERY_FAST_MEDIAN_MODE

// Comment back in #define GET_PATH_FROM_FILESYSTEM to use filesystem instead of default to fetch .exe path
// If you're using filesystem, split.exe has to be in the same folder as the solution, not the .exe
// Filesystem is still used for some things even if this is commented out
//#define GET_PATH_FROM_FILESYSTEM

// Comment back in #define SHR_DIV to override C++ compiler and use shr when dividing unsigned ints by 2 instead of sar
// Weirdly, using the asm shr to divide by 2 is faster in debug mode, but not in release mode, so be wary about enabling this (I think that the compiler automatically uses it in asm and the 'mov eax, ecx' call is just extra there
// Also, performance effects of this matter mainly if you're using fast median/mode and matter more if the images have more similar points
//#define SHR_DIV

// Comment back in #define NO_PIXEL_STRUCT to write raw data to the file instead of using the Pixel struct
//#define NO_PIXEL_STRUCT

// Comment back in #define TRY_MULTITHREAD_LOAD to try to multithread array loading, which doesn't really work
// It is now automatically doing this, so don't bother turning this on (I should delete it)
//#define TRY_MULTITHREAD_LOAD

// Comment back in #define PRINT_TIME_AFTER_FILESYSTEM to print time after the program finds valid files
//#define PRINT_TIME_AFTER_FILESYSTEM

// Comment back in #define PRINT_TIME_BEFORE_WRITING to print time after we process data, but before we write it back to the files
//#define PRINT_TIME_BEFORE_WRITING

// Comment back in #define WRITE_HEADER_SEPERATELY to write .bmp output file's headers seperately
// You would think it would be faster with this enabled, but it's actually not...
//#define WRITE_HEADER_SEPERATELY

// Only include iostream if we're actually using it, as it takes a little bit of build time
#ifdef PRINT_PIXELS_PER_SECOND
#include <iostream>
#else // PRINT_PIXELS_PER_SECOND
#ifdef PRINT_NUM_MEANS
#include <iostream>
#else // PRINT_NUM_MEANS
#ifdef PRINT_NUM_MEDIANS
#include <iostream>
#else // PRINT_NUM_MEDIANS
#ifdef PRINT_NUM_MODES
#include <iostream>
#else // PRINT_NUM_MODES
#ifdef PRINT_TIME_AFTER_FILESYSTEM
#include <iostream>
#else // PRINT_TIME_AFTER_FILESYSTEM
#ifdef PRINT_TIME_BEFORE_WRITING
#include <iostream>
#endif // PRINT_TIME_BEFORE_WRITING
#endif // PRINT_TIME_AFTER_FILESYSTEM
#endif // PRINT_NUM_MODES
#endif // PRINT_NUM_MEDIANS
#endif // PRINT_NUM_MEANS
#endif // PRINT_PIXELS_PER_SECOND

// Image class, don't initialize things as Images, but it's useful (and saves some code) to initialize Image children as Image* blah = new BitmapImage;, for example
class Image {
public:
#ifndef NO_PIXEL_STRUCT
    // Pixel struct that can write to .bmp files and saves pixel data
    struct Pixel {
        uint8_t blue;
        uint8_t green;
        uint8_t red;

        void save_on_file(std::ofstream&);
    };
#endif // NO_PIXEL_STRUCT

    // BMP Header information, don't worry about this
    struct BmpHeader {
        char bitmapSignatureBytes[2] = { 'B', 'M' };
        uint32_t sizeOfBitmapFile;
        uint32_t reservedBytes = 0;
        uint32_t pixelDataOffset = 54;

        void save_on_file(std::ofstream&);
    };

    // Other things that we have to write to the bmp header, don't worry about this
    struct BmpInfoHeader {
        uint32_t sizeOfThisHeader = 40;
        int32_t width; // in pixels
        int32_t height; // in pixels
        uint16_t numberOfColorPlanes = 1;
        uint16_t colorDepth = 24;
        uint32_t compressionMethod = 0;
        uint32_t rawBitmapDataSize = 0;
        int32_t horizontalResolution = 3780; // in pixel per meter
        int32_t verticalResolution = 3780; // in pixel per meter
        uint32_t colorTableEntries = 0;
        uint32_t importantColors = 0;

        void save_on_file(std::ofstream&);
    };

    // I'm using an overloaded constructor so that you don't have to specify output paths if you don't want to
    // You can also add an int at the end to not have it add path to output files
    Image(std::string);
    Image(std::string, const char*, const char*, const char*);
    Image(std::string, const char*, const char*, const char*, int);
    Image(COLORREF***, const char*, const char*, const char*, int);
    // I like overriding the default destructor, even though I'm not actually destructing anything
    ~Image();

private:
    // If any of the print options are on, define variables that can save the print data
#ifdef PRINT_PIXELS_PER_SECOND
    int m_pixels;
    clock_t readTime;
#endif // PRINT_PIXELS_PER_SECOND
#ifdef PRINT_NUM_MEANS
    int means;
    clock_t meanTime;
#endif // PRINT_NUM_MEANS
#ifdef PRINT_NUM_MEDIANS
    int medians;
    clock_t medianTime;
#endif // PRINT_NUM_MEDIANS
#ifdef PRINT_NUM_MODES
    int modes;
    clock_t modeTime;
#endif // PRINT_NUM_MODES

    // Variables for the directory path and output paths
    std::string path;
#ifndef NO_MEAN
    std::string meanOutput;
#endif // NO_MEAN
#ifndef NO_MEDIAN
    std::string medianOutput;
#endif // NO_MEDIAN
#ifndef NO_MODE
    std::string modeOutput;
#endif // NO_MODE

    // ReadImage should be called in the ActualClean routine
    virtual COLORREF** ReadImage(WCHAR*, int&, int&);

public:
    COLORREF*** optional_picture;
    int optional_num_pics;

    // I'm not bothering to define the toggled off mean/median/mode versions
#ifndef NO_MEAN
    COLORREF mean(COLORREF***, int, int, int);
#endif // NO_MEAN
#ifndef NO_MEDIAN
#ifdef BASIC_FAST_MEDIAN_MODE
    COLORREF basic_fast_median(COLORREF***, int, int, const int);
#else // BASIC_FAST_MEDIAN_MODE
#ifdef VERY_FAST_MEDIAN_MODE
    COLORREF very_fast_median(COLORREF***, int, int, const int);
#else // VERY_FAST_MEDIAN_MODE
    COLORREF median(COLORREF***, int, int, const int);
#endif // VERY_FAST_MEDIAN_MODE
#endif // BASIC_FAST_MEDIAN_MODE
#endif // NO_MEDIAN
#ifndef NO_MODE
#ifdef BASIC_FAST_MEDIAN_MODE
    COLORREF basic_fast_mode(COLORREF***, int, int, const int);
#else // BASIC_FAST_MEDIAN_MODE
#ifdef VERY_FAST_MEDIAN_MODE
    COLORREF very_fast_mode(COLORREF***, int, int, const int);
#else // VERY_FAST_MEDIAN_MODE
    COLORREF mode(COLORREF***, int, int, const int);
#endif // VERY_FAST_MEDIAN_MODE
#endif // BASIC_FAST_MEDIAN_MODE
#endif // NO_MODE

private:
    // Clean(int&, int&) and Clean() both call this
    virtual void ActualClean(int&, int&);

public:
    // These are literally the same function, except Clean(int&, int&) allows you to retrieve width/height data
    void Clean(int&, int&);
    void Clean();

    // If any of the print options are on, define basic functions that allow child classes to set/retrieve the print data
#ifdef PRINT_PIXELS_PER_SECOND
    void SetReadTime(clock_t);
    void SetPixels(int);

    clock_t GetReadTime();
    int GetPixels();
#endif // PRINT_PIXELS_PER_SECOND
#ifdef PRINT_NUM_MEANS
    void SetMeanTime(clock_t);
    void SetMeans(int);

    clock_t GetMeanTime();
    int GetMeans();
#endif // PRINT_NUM_MEANS
#ifdef PRINT_NUM_MEDIANS
    void SetMedianTime(clock_t);
    void SetMedians(int);

    clock_t GetMedianTime();
    int GetMedians();
#endif // PRINT_NUM_MEDIANS
#ifdef PRINT_NUM_MODES
    void SetModeTime(clock_t);
    void SetModes(int);

    clock_t GetModeTime();
    int GetModes();
#endif // PRINT_NUM_MODES

    // Also define functions that let the child classes get the path data
    std::string GetPath();
#ifndef NO_MEAN
    std::string GetMeanOutput();
#endif // NO_MEAN
#ifndef NO_MEDIAN
    std::string GetMedianOutput();
#endif // NO_MEDIAN
#ifndef NO_MODE
    std::string GetModeOutput();
#endif // NO_MODE
};

// Child class BitmapImage, used for .bmp images. It is slightly faster than JPEGImage
class BitmapImage : public Image {
    // Use Image's constructor
    using Image::Image;
private:
    // I could have added these to Image, but I think it's cleaner like this
    BmpHeader bmpHeader;
    BmpInfoHeader bmpInfoHeader;

public:
    // Override both of the virtual functions in Image
    COLORREF** ReadImage(WCHAR*, int&, int&) override;

    void ActualClean(int&, int&) override;
};

// Child class JPEGImage, used for .jpg images. It is slightly slower than BitmapImage, mainly due to file compression/etc. on jpegs
class JPEGImage : public Image {
    // Again, use Image's constructor
    using Image::Image;
private:
    BmpHeader bmpHeader;
    BmpInfoHeader bmpInfoHeader;

public:
    // Also override both of the virtual functions
    COLORREF** ReadImage(WCHAR*, int&, int&) override;

    void ActualClean(int&, int&) override;
};

// Video is a bit different than the other image classes
class Video {
private:
    // BMP Header information, don't worry about this
    struct BmpHeader {
        char bitmapSignatureBytes[2] = { 'B', 'M' };
        uint32_t sizeOfBitmapFile;
        uint32_t reservedBytes = 0;
        uint32_t pixelDataOffset = 54;

        void save_on_file(std::ofstream&);
    };

    // Other things that we have to write to the bmp header, don't worry about this
    struct BmpInfoHeader {
        uint32_t sizeOfThisHeader = 40;
        int32_t width; // in pixels
        int32_t height; // in pixels
        uint16_t numberOfColorPlanes = 1;
        uint16_t colorDepth = 24;
        uint32_t compressionMethod = 0;
        uint32_t rawBitmapDataSize = 0;
        int32_t horizontalResolution = 3780; // in pixel per meter
        int32_t verticalResolution = 3780; // in pixel per meter
        uint32_t colorTableEntries = 0;
        uint32_t importantColors = 0;

        void save_on_file(std::ofstream&);
    };

    BmpHeader bmpHeader;
    BmpInfoHeader bmpInfoHeader;

public:
    Video(std::string, std::string, std::string);
    Video(std::string, std::string, std::string, std::string, std::string, std::string);
    ~Video();

private:
    std::string path, ffmpegPath;
#ifndef NO_MEAN
    std::string meanOutput;
#endif // NO_MEAN
#ifndef NO_MEDIAN
    std::string medianOutput;
#endif // NO_MEDIAN
#ifndef NO_MODE
    std::string modeOutput;
#endif // NO_MODE
    std::string filename;

    void ActualClean(int&, int&);
#ifndef GET_PATH_FROM_FILESYSTEM
    std::string GetEXEName();
#endif // GET_PATH_FROM_FILESYSTEM

public:
    void Clean(int&, int&);
    void Clean();
};

#ifdef SHR_DIV
// extern "C" as quick_div is a C-style asm function. Function definition is in quick_div.asm
// This needs to be inlined to do anything
extern "C" __forceinline int quick_div(int thingToDivide);
#endif // SHR_DIV

// If you're debugging, change Project > Properties > C/C++ > Code Generation > Basic Runtime Checks to /RTC1 and Project > Properties > C/C++ > Optimization > Optimization to \Od

COLORREF getPixel(AVFrame*, short, short);
bool decode(AVCodecContext*, AVFrame*, AVPacket*, COLORREF***, int, int, int, uint8_t*, AVFrame*);
