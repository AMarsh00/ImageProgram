#include "image_cleaner.h"
#include <iostream>

int main() {
	std::cout << clock() << std::endl;

	BitmapImage image("D:\\ImageProgram\\");
	// BitmapImage image("D:\\ImageProgram\\", "customMean.bmp", "customMedian.bmp", "customMode.bmp");
	// You can also set the output files (this example will put the mean image in D:\\ImageProgram\\customMean.bmp)
	
	image.Clean();
	// image.Clean(width, height);
	// You can also have it return width and height by passing in width and height

	std::cout << clock() << std::endl;

	JPEGImage image2("D:\\ImageProgram\\", "JPEG\\jpeg_mean.bmp", "JPEG\\jpeg_median.bmp", "JPEG\\jpeg_mode.bmp");
	// JPEGImage image2("D:\\ImageProgram\\");
	// You can also not set specific output files, but if you're using multiple instances of an Image child, you want to
	// Do note that all outputs must be .bmp files, as I haven't figured out how to write to JPG/GIF/etc. yet
	//TODO: CHANGE THAT^^
	// As they are all bitmaps, MAKE SURE you're outputting in a DIFFERENT folder for each Image child, otherwise it might be a bit funky...

	image2.Clean();
	// image2.Clean(width, height);
	// You can also have it return width and height by passing them in

	std::cout << clock() << std::endl;
}