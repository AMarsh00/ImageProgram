#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <thread>

#include "stb_image.h"

#define FAST_MEDIAN_RANGE 5 // Fast median will return average if max is within FAST_MEDIAN_RANGE of the min
#define FAST_MODE_RANGE 5 // Fast mode will return average if max is within FAST_MODE_RANGE of the min

// Comment back in #define CHECK_BAD_ALLOC to break if malloc or calloc fails (debug feature)
//#define CHECK_BAD_ALLOC

// Comment back in #define NO_MEAN, #define NO_MEDIAN, or #define NO_MODE to not do those
//#define NO_MEAN
//#define NO_MEDIAN
//#define NO_MODE

// Comment back in #define PRINT_PIXELS_PER_SECOND, #define PRINT_NUM_MEANS, #define PRINT_NUM_MEDIANS, #define PRINT_NUM_MODES to print those things
#define PRINT_PIXELS_PER_SECOND
#define PRINT_NUM_MEANS
#define PRINT_NUM_MEDIANS
#define PRINT_NUM_MODES

// Comment back in #define PRETTY_PRINT to pretty print outputs (only relevant if the above output macros are commented in, and will take a little overhead)
#define PRETTY_PRINT

// Comment back in #define FAST_MEDIAN_MODE to use a faster, but slightly less accurate, median and mode function
#define FAST_MEDIAN_MODE

#ifdef PRINT_PIXELS_PER_SECOND
#include <iostream>
#else
#ifdef PRINT_NUM_MEANS
#include <iostream>
#else
#ifdef PRINT_NUM_MEDIANS
#include <iostream>
#else
#ifdef PRINT_NUM_MODES
#include <iostream>
#endif
#endif
#endif
#endif

class Image {
public:
    struct Pixel {
        uint8_t blue;
        uint8_t green;
        uint8_t red;

        void save_on_file(std::ofstream&);
    };

    struct BmpHeader {
        char bitmapSignatureBytes[2] = { 'B', 'M' };
        uint32_t sizeOfBitmapFile;
        uint32_t reservedBytes = 0;
        uint32_t pixelDataOffset = 54;

        void save_on_file(std::ofstream&);
    };

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

    Image(std::string);
    Image(std::string, const char*, const char*, const char*);
    ~Image();

private:
#ifdef PRINT_PIXELS_PER_SECOND
    int m_pixels;
    clock_t readTime;
#endif
#ifdef PRINT_NUM_MEANS
    int means;
    clock_t meanTime;
#endif
#ifdef PRINT_NUM_MEDIANS
    int medians;
    clock_t medianTime;
#endif
#ifdef PRINT_NUM_MODES
    int modes;
    clock_t modeTime;
#endif

    std::string path;
#ifndef NO_MEAN
    std::string meanOutput;
#endif
#ifndef NO_MEDIAN
    std::string medianOutput;
#endif
#ifndef NO_MODE
    std::string modeOutput;
#endif

    virtual COLORREF** ReadImage(WCHAR*, int&, int&);

public:
#ifndef NO_MEAN
    COLORREF mean(COLORREF***, int, int, int);
#endif
#ifndef NO_MEDIAN
    COLORREF median(COLORREF***, int, int, const int);
    COLORREF fast_median(COLORREF***, int, int, const int);
#endif
#ifndef NO_MODE
    COLORREF mode(COLORREF***, int, int, const int);
    COLORREF fast_mode(COLORREF***, int, int, const int);
#endif

private:
    virtual void ActualClean(int&, int&);

public:
    void Clean(int&, int&);
    void Clean();

#ifdef PRINT_PIXELS_PER_SECOND
    void SetReadTime(clock_t);
    void SetPixels(int);

    clock_t GetReadTime();
    int GetPixels();
#endif
#ifdef PRINT_NUM_MEANS
    void SetMeanTime(clock_t);
    void SetMeans(int);

    clock_t GetMeanTime();
    int GetMeans();
#endif
#ifdef PRINT_NUM_MEDIANS
    void SetMedianTime(clock_t);
    void SetMedians(int);

    clock_t GetMedianTime();
    int GetMedians();
#endif
#ifdef PRINT_NUM_MODES
    void SetModeTime(clock_t);
    void SetModes(int);

    clock_t GetModeTime();
    int GetModes();
#endif

    std::string GetPath();
#ifndef NO_MEAN
    std::string GetMeanOutput();
#endif
#ifndef NO_MEDIAN
    std::string GetMedianOutput();
#endif
#ifndef NO_MODE
    std::string GetModeOutput();
#endif
};

class BitmapImage : public Image {
    using Image::Image;
private:
    BmpHeader bmpHeader;
    BmpInfoHeader bmpInfoHeader;

public:
    COLORREF** ReadImage(WCHAR*, int&, int&) override;

    void ActualClean(int&, int&) override;
};

class JPEGImage : public Image {
    using Image::Image;
private:
    BmpHeader bmpHeader;
    BmpInfoHeader bmpInfoHeader;

public:
    COLORREF** ReadImage(WCHAR*, int&, int&) override;

    void ActualClean(int&, int&) override;
};