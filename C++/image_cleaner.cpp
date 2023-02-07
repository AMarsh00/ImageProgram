#include "image_cleaner.h"

#ifndef NO_PIXEL_STRUCT
// Routine to save the data from Pixel onto an ofstream& type file (basically just write to a file)
void Image::Pixel::save_on_file(std::ofstream& fout) {
	//fout.write((char*)&this->blue, sizeof(uint8_t));
	//fout.write((char*)&this->green, sizeof(uint8_t));
	//fout.write((char*)&this->red, sizeof(uint8_t));
	// The line below basically does all of that ^ at once (one write call instead of 3, which is a big improvement as we're calling this function millions of times)
	// Weirdly, we have to save them backwards
	fout.write((char*)((std::string)(char*)&this->blue + (std::string)(char*)&this->green + (std::string)(char*)&this->red).c_str(), 3 * sizeof(uint8_t));
}
#endif // NO_PIXEL_STRUCT

// Image constructor, initialize everything
Image::Image(std::string newPath) {
#ifdef PRINT_PIXELS_PER_SECOND
	m_pixels = 0;
	readTime = 0;
#endif // PRINT_PIXELS_PER_SECOND
#ifdef PRINT_NUM_MEANS
	means = 0;
	meanTime = 0;
#endif // PRINT_NUM_MEANS
#ifdef PRINT_NUM_MEDIANS
	medians = 0;
	medianTime = 0;
#endif // PRINT_NUM_MEDIANS
#ifdef PRINT_NUM_MODES
	modes = 0;
	modeTime = 0;
#endif // PRINT_NUM_MODES
	path = newPath;

#ifndef NO_MEAN
	std::string meanPath = path;
	meanPath.append("mean.bmp");
	meanOutput = meanPath;
#endif // NO_MEAN
#ifndef NO_MEDIAN
	std::string medianPath = path;
	medianPath.append("median.bmp");
	medianOutput = medianPath;
#endif // NO_MEDIAN
#ifndef NO_MODE
	std::string modePath = path;
	modePath.append("mode.bmp");
	modeOutput = modePath;
#endif // NO_MODE
}

// Overloaded Image constructor, this one initializes variables using custom output names
Image::Image(std::string newPath, const char* newMeanOutput, const char* newMedianOutput, const char* newModeOutput) {
#ifdef PRINT_PIXELS_PER_SECOND
	m_pixels = 0;
	readTime = 0;
#endif // PRINT_PIXELS_PER_SECOND
#ifdef PRINT_NUM_MEANS
	means = 0;
	meanTime = 0;
#endif // PRINT_NUM_MEANS
#ifdef PRINT_NUM_MEDIANS
	medians = 0;
	medianTime = 0;
#endif // PRINT_NUM_MEDIANS
#ifdef PRINT_NUM_MODES
	modes = 0;
	modeTime = 0;
#endif // PRINT_NUM_MODES
	path = newPath;

#ifndef NO_MEAN
	std::string meanPath = path;
	meanPath.append(newMeanOutput);
	meanOutput = meanPath;
#endif // NO_MEAN
#ifndef NO_MEDIAN
	std::string medianPath = path;
	medianPath.append(newMedianOutput);
	medianOutput = medianPath;
#endif // NO_MEDIAN
#ifndef NO_MODE
	std::string modePath = path;
	modePath.append(newModeOutput);
	modeOutput = modePath;
#endif // NO_MODE

	optional_picture = nullptr;
	optional_num_pics = 0;
}

// Overloaded Image constructor, this one initializes variables using custom output names
// This one doesn't add path to the output files
Image::Image(std::string newPath, const char* newMeanOutput, const char* newMedianOutput, const char* newModeOutput, int) {
#ifdef PRINT_PIXELS_PER_SECOND
	m_pixels = 0;
	readTime = 0;
#endif // PRINT_PIXELS_PER_SECOND
#ifdef PRINT_NUM_MEANS
	means = 0;
	meanTime = 0;
#endif // PRINT_NUM_MEANS
#ifdef PRINT_NUM_MEDIANS
	medians = 0;
	medianTime = 0;
#endif // PRINT_NUM_MEDIANS
#ifdef PRINT_NUM_MODES
	modes = 0;
	modeTime = 0;
#endif // PRINT_NUM_MODES
	path = newPath;

#ifndef NO_MEAN
	meanOutput = newMeanOutput;
#endif // NO_MEAN
#ifndef NO_MEDIAN
	medianOutput = newMedianOutput;
#endif // NO_MEDIAN
#ifndef NO_MODE
	modeOutput = newModeOutput;
#endif // NO_MODE

	optional_picture = nullptr;
	optional_num_pics = 0;
}

// Overloaded Image constructor, this one initializes variables using custom output names
// This one is used exclusively in Video class
Image::Image(COLORREF*** pictures, const char* newMeanOutput, const char* newMedianOutput, const char* newModeOutput, int newNumPics) {
#ifdef PRINT_PIXELS_PER_SECOND
	m_pixels = 0;
	readTime = 0;
#endif // PRINT_PIXELS_PER_SECOND
#ifdef PRINT_NUM_MEANS
	means = 0;
	meanTime = 0;
#endif // PRINT_NUM_MEANS
#ifdef PRINT_NUM_MEDIANS
	medians = 0;
	medianTime = 0;
#endif // PRINT_NUM_MEDIANS
#ifdef PRINT_NUM_MODES
	modes = 0;
	modeTime = 0;
#endif // PRINT_NUM_MODES

	optional_picture = pictures;
	optional_num_pics = newNumPics;

#ifndef NO_MEAN
	meanOutput = newMeanOutput;
#endif // NO_MEAN
#ifndef NO_MEDIAN
	medianOutput = newMedianOutput;
#endif // NO_MEDIAN
#ifndef NO_MODE
	modeOutput = newModeOutput;
#endif // NO_MODE
}

// Image destructor. I have nothing to delete, so it's empty
Image::~Image() {
	
}

// I'm overriding this anyways, so it doesn't matter what's in it
COLORREF** Image::ReadImage(WCHAR*, int&, int&) {
	return nullptr;// Virtual, so don't care
}

// Define a mean routine if that's enabled
#ifndef NO_MEAN
COLORREF Image::mean(COLORREF*** colour, int x, int y, int num) {
	float red = 0, green = 0, blue = 0;

	// Index through all of the lists, add to the RGB totals
	for (int i = 0; i < num; i++) {
		COLORREF** cur = colour[i];
		red += GetRValue(cur[y][x]);
		green += GetGValue(cur[y][x]);
		blue += GetBValue(cur[y][x]);
	}

	// If mean printing is enabled, increase that index
#ifdef PRINT_NUM_MEANS
	means++;
#endif // PRINT_NUM_MEANS
	
	// Return total / total number (just the basic mean)
	return RGB(red / (float)num, green / (float)num, blue / (float)num);
}
#endif // NO_MEAN

#ifndef NO_MEDIAN
// Basic fast median will just return average if we're within that range, otherwise it will return true median
// Only bother defining it if that mode is on
#ifdef BASIC_FAST_MEDIAN_MODE
COLORREF Image::basic_fast_median(COLORREF*** colour, int x, int y, const int num) {
	// Allocate enough memory for three arrays of ints with size num
	int* redList = static_cast<int*>(malloc(num * sizeof(int)));
	int* greenList = static_cast<int*>(malloc(num * sizeof(int)));
	int* blueList = static_cast<int*>(malloc(num * sizeof(int)));

	// If CHECK_BAD_ALLOC is defined, break on bad allowance (debug feature)
#ifdef CHECK_BAD_ALLOC
	if (redList == nullptr) {
		quick_exit(1);
	}
	if (greenList == nullptr) {
		quick_exit(1);
	}
	if (blueList == nullptr) {
		quick_exit(1);
	}
#endif // CHECK_BAD_ALLOC

	// Convert colour into red/green/blueLists
	for (int i = 0; i < num; i++) {
		redList[i] = GetRValue(colour[i][y][x]);
		greenList[i] = GetGValue(colour[i][y][x]);
		blueList[i] = GetBValue(colour[i][y][x]);
	}

	// Get max/min of everything
	int redMax = redList[0], redMin = redList[0];
	int greenMax = greenList[0], greenMin = greenList[0];
	int blueMax = blueList[0], blueMin = blueList[0];
	for (int i = 1; i < num; i++) {
		redMax = max(redMax, redList[i]);
		greenMax = max(greenMax, greenList[i]);
		blueMax = max(blueMax, blueList[i]);

		redMin = min(redMin, redList[i]);
		greenMin = min(greenMin, greenList[i]);
		blueMin = min(blueMin, blueList[i]);
	}

	int redVal, greenVal, blueVal;

	// If we're within the designated range, just return average
	if (redMax <= redMin + BASIC_FAST_MEDIAN_RANGE) {
#ifdef SHR_DIV
		redVal = quick_div(redMin + redMax);
#else // SHR_DIV
		redVal = (redMin + redMax) / 2;
#endif // SHR_DIV
	}
	// Otherwise, return true median
	else {
		// Sort red list, then take the middle value
		std::sort(redList, redList + num, std::greater<int>());

		if (num % 2 == 1) {
#ifdef SHR_DIV
			redVal = redList[quick_div(num + 1)];
#else // SHR_DIV
			redVal = redList[(num + 1) / 2];
#endif // SHR_DIV
		}
		else {
#ifdef SHR_DIV
			redVal = quick_div(redList[quick_div(num)] + redList[quick_div(num) + 1]);
#else // SHR_DIV
			redVal = (redList[num / 2] + redList[num / 2 + 1]) / 2;
#endif // SHR_DIV
		}
	}

	// Do the same as above with greenList, blueList
	if (greenMax <= greenMin + BASIC_FAST_MEDIAN_RANGE) {
#ifdef SHR_DIV
		greenVal = quick_div(greenMin + greenMax);
#else // SHR_DIV
		greenVal = (greenMin + greenMax) / 2;
#endif // SHR_DIV
	}
	else {
		std::sort(greenList, greenList + num, std::greater<int>());

		if (num % 2 == 1) {
#ifdef SHR_DIV
			greenVal = greenList[quick_div(num + 1)];
#else // SHR_DIV
			greenVal = greenList[(num + 1) / 2];
#endif // SHR_DIV
		}
		else {
#ifdef SHR_DIV
			greenList = quick_div(greenList[quick_div(num)] + greenList[quick_div(num) + 1]);
#else // SHR_DIV
			greenVal = (greenList[num / 2] + greenList[num / 2 + 1]) / 2;
#endif // SHR_DIV
		}
	}

	if (blueMax <= blueMin + BASIC_FAST_MEDIAN_RANGE) {
#ifdef SHR_DIV
		blueVal = quick_div(blueMin + blueMax);
#else // SHR_DIV
		blueVal = (blueMin + blueMax) / 2;
#endif // SHR_DIV
	}
	else {
		std::sort(blueList, blueList + num, std::greater<int>());

		if (num % 2 == 1) {
#ifdef SHR_DIV
			blueVal = blueList[quick_div(num + 1)];
#else // SHR_DIV
			blueVal = blueList[(num + 1) / 2];
#endif // SHR_DIV
		}
		else {
#ifdef SHR_DIV
			blueVal = quick_div(blueList[quick_div(num)] + blueList[quick_div(num) + 1]);
#else // SHR_DIV
			blueVal = (blueList[num / 2] + blueList[num / 2 + 1]) / 2;
#endif // SHR_DIV
		}
	}

	// We're not using these lists anymore, so free any memory allocated to them
	free(redList);
	free(greenList);
	free(blueList);

	// If the median printing feature is turned on, increase that index
#ifdef PRINT_NUM_MEDIANS
	medians++;
#endif // PRINT_NUM_MEDIANS

	// Make sure to cast to a COLORREF type structure before returning
	return RGB(redVal, greenVal, blueVal);
}

#else // BASIC_FAST_MEDIAN_MODE
// If VERY_FAST_MEDIAN_MODE is turned on, define that median. This one uses quick_select to try to get a much faster median without being too innaccurate
#ifdef VERY_FAST_MEDIAN_MODE
COLORREF Image::very_fast_median(COLORREF*** colour, int x, int y, const int num) {
	// Allocate enough memory for three lists of ints with size num
	int* redList = static_cast<int*>(malloc(num * sizeof(int)));
	int* greenList = static_cast<int*>(malloc(num * sizeof(int)));
	int* blueList = static_cast<int*>(malloc(num * sizeof(int)));

	// If CHECK_BAD_ALLOC is defined, break on bad allowance (debug feature)
#ifdef CHECK_BAD_ALLOC
	if (redList == nullptr) {
		quick_exit(1);
	}
	if (greenList == nullptr) {
		quick_exit(1);
	}
	if (blueList == nullptr) {
		quick_exit(1);
	}
#endif // CHECK_BAD_ALLOC

	// Turn colour into RGB arrays
	for (int i = 0; i < num; i++) {
		redList[i] = GetRValue(colour[i][y][x]);
		greenList[i] = GetGValue(colour[i][y][x]);
		blueList[i] = GetBValue(colour[i][y][x]);
	}

	// Use quick_select to find the median fast
	int redVal = quick_select<int>(redList, num);	
	int greenVal = quick_select<int>(greenList, num);
	int blueVal = quick_select<int>(blueList, num);

	// If median printing is on, increase that index
#ifdef PRINT_NUM_MEDIANS
	medians++;
#endif // PRINT_NUM_MEDIANS

	// Free any memory allocated to the arrays
	free(redList);
	free(greenList);
	free(blueList);

	// Make sure to cast to a COLORREF before returning
	return RGB(redVal, greenVal, blueVal);
}

// If neither of those are defined, go with a default median that just finds the true median
#else // VERY_FAST_MEDIAN_MODE
COLORREF Image::median(COLORREF*** colour, int x, int y, const int num) {
	// Allocate enough memory for three lists of ints, with size num
	int* redList = static_cast<int*>(malloc(num * sizeof(int)));
	int* greenList = static_cast<int*>(malloc(num * sizeof(int)));
	int* blueList = static_cast<int*>(malloc(num * sizeof(int)));

	// Break on bad allowance (debug feature)
#ifdef CHECK_BAD_ALLOC
	if (redList == nullptr) {
		quick_exit(1);
	}
	if (greenList == nullptr) {
		quick_exit(1);
	}
	if (blueList == nullptr) {
		quick_exit(1);
	}
#endif // CHECK_BAD_ALLOC

	// Convert colour into RGB arrays
	for (int i = 0; i < num; i++) {
		redList[i] = GetRValue(colour[i][y][x]);
		greenList[i] = GetGValue(colour[i][y][x]);
		blueList[i] = GetBValue(colour[i][y][x]);
	}

	// Sort the arrays, then pick the middle value to return
	std::sort(redList, redList + num, std::greater<int>());
	std::sort(greenList, greenList + num, std::greater<int>());
	std::sort(blueList, blueList + num, std::greater<int>());

	int redVal, greenVal, blueVal;
	if (num % 2 == 1) {
#ifdef SHR_DIV
		redVal = redList[quick_div(num + 1)];
		greenVal = greenList[quick_div(num + 1)];
		blueVal = blueList[quick_div(num + 1)];
#else // SHR_DIV
		redVal = redList[(num + 1) / 2];
		greenVal = greenList[(num + 1) / 2];
		blueVal = blueList[(num + 1) / 2];
#endif // SHR_DIV
	}
	else {
#ifdef SHR_DIV
		redVal = quick_div(redList[quick_div(num)] + redList[quick_div(num) + 1]);
		greenVal = quick_div(greenList[quick_div(num)] + greenList[quick_div(num) + 1]);
		blueVal = quick_div(blueList[quick_div(num)] + blueList[quick_div(num) + 1]);
#else // SHR_DIV
		redVal = (redList[num / 2] + redList[num / 2 + 1]) / 2;
		greenVal = (greenList[num / 2] + greenList[num / 2 + 1]) / 2;
		blueVal = (blueList[num / 2] + blueList[num / 2 + 1]) / 2;
#endif // SHR_DIV
	}

	// Free any memory allocated to the RGB arrays
	free(redList);
	free(greenList);
	free(blueList);

	// If median printing is on, increase that index
#ifdef PRINT_NUM_MEDIANS
	medians++;
#endif // PRINT_NUM_MEDIANS

	// Make sure to cast to COLORREF before returning
	return RGB(redVal, greenVal, blueVal);
}
#endif // VERY_FAST_MEDIAN_MODE
#endif // BASIC_FAST_MEDIAN_MODE
#endif // NO_MEDIAN

#ifndef NO_MODE
// If we're using basic fast mode, define that. It just returns the average if min/max are within a certain range, and returns the true mode otherwise
#ifdef BASIC_FAST_MEDIAN_MODE
COLORREF Image::basic_fast_mode(COLORREF*** colour, int x, int y, const int num) {
	// Allocate enough memory for three arrays of ints, with size num
	int* redList = static_cast<int*>(malloc(num * sizeof(int)));
	int* greenList = static_cast<int*>(malloc(num * sizeof(int)));
	int* blueList = static_cast<int*>(malloc(num * sizeof(int)));

	// Break on bad allowance (debug feature)
#ifdef CHECK_BAD_ALLOC
	if (redList == nullptr) {
		quick_exit(1);
	}
	if (greenList == nullptr) {
		quick_exit(1);
	}
	if (blueList == nullptr) {
		quick_exit(1);
	}
#endif // CHECK_BAD_ALLOC

	// Convert colour into RGB arrays
	for (int i = 0; i < num; i++) {
		redList[i] = GetRValue(colour[i][y][x]);
		greenList[i] = GetGValue(colour[i][y][x]);
		blueList[i] = GetBValue(colour[i][y][x]);
	}

	// Figure out min/max of each colour
	// When these were unsigned, the compiler still didn't give me shr :(
	int redMode = *redList;
	int greenMode = *greenList;
	int blueMode = *blueList;

	int redMax = redList[0], redMin = redList[0];
	int greenMax = greenList[0], greenMin = greenList[0];
	int blueMax = blueList[0], blueMin = blueList[0];
	for (int i = 1; i < num; i++) {
		redMax = max(redMax, redList[i]);
		greenMax = max(greenMax, greenList[i]);
		blueMax = max(blueMax, blueList[i]);

		redMin = min(redMin, redList[i]);
		greenMin = min(greenMin, greenList[i]);
		blueMin = min(blueMin, blueList[i]);
	}

	// If all of the mins and maxes are close, just return the average
	if (redMax <= redMin + BASIC_FAST_MODE_RANGE && greenMax <= greenMin + BASIC_FAST_MODE_RANGE && blueMax <= blueMin + BASIC_FAST_MODE_RANGE) {
#ifdef SHR_DIV
		redMode = quick_div(redMin + redMax);
		greenMode = quick_div(greenMin + greenMax);
		blueMode = quick_div(blueMin + blueMax);
#else // SHR_DIV
		redMode = (redMin + redMax) / 2;
		greenMode = (greenMin + greenMax) / 2;
		blueMode = (blueMin + blueMax) / 2;
#endif // SHR_DIV
	}
	// Otherwise, return the true mode
	else {
		// Sort all arrays
		std::sort(redList, redList + num, std::greater<int>());
		std::sort(greenList, greenList + num, std::greater<int>());
		std::sort(blueList, blueList + num, std::greater<int>());

		// This is a bit complicated, but the rest of this else block just finds the mode
		int red_count = 1, max_redCount = 1;
		int green_count = 1, max_greenCount = 1;
		int blue_count = 1, max_blueCount = 1;
		for (int i = 1; i < num; i++) {
			if (redList[i] == redList[i - 1]) {
				red_count++;
			}
			else {
				if (red_count > max_redCount) {
					max_redCount = red_count;
					redMode = redList[i - 1];
				}
				red_count = 1;
			}

			if (greenList[i] == greenList[i - 1]) {
				green_count++;
			}
			else {
				if (green_count > max_greenCount) {
					max_greenCount = green_count;
					greenMode = greenList[i - 1];
				}
				green_count = 1;
			}

			if (blueList[i] == blueList[i - 1]) {
				blue_count++;
			}
			else {
				if (blue_count > max_blueCount) {
					max_blueCount = blue_count;
					blueMode = blueList[i - 1];
				}
				blue_count = 1;
			}
		}

		if (red_count > max_redCount) {
			redMode = redList[num - 1];
		}
		if (blue_count > max_blueCount) {
			blueMode = blueList[num - 1];
}
		if (green_count > max_greenCount) {
			greenMode = greenList[num - 1];
		}
	}

	// Cast return value to a COLORREF
	COLORREF ret = RGB(redMode, greenMode, blueMode);

	// Free any memory allocated to the arrays
	free(redList);
	free(greenList);
	free(blueList);

	// If mode printing is on, increase that index
#ifdef PRINT_NUM_MODES
	modes++;
#endif // PRINT_NUM_MODES

	return ret;
}

#else // BASIC_FAST_MEDIAN_MODE
// If we're using very_fast_mode, turn that on
// I couldn't figure out a fast one, so it is literally the EXACT same as basic_fast_mode
#ifdef VERY_FAST_MEDIAN_MODE
COLORREF Image::very_fast_mode(COLORREF*** colour, int x, int y, const int num) {
	int* redList = static_cast<int*>(malloc(num * sizeof(int)));
	int* greenList = static_cast<int*>(malloc(num * sizeof(int)));
	int* blueList = static_cast<int*>(malloc(num * sizeof(int)));

#ifdef CHECK_BAD_ALLOC
	if (redList == nullptr) {
		quick_exit(1);
	}
	if (greenList == nullptr) {
		quick_exit(1);
	}
	if (blueList == nullptr) {
		quick_exit(1);
	}
#endif // CHECK_BAD_ALLOC

	for (int i = 0; i < num; i++) {
		redList[i] = GetRValue(colour[i][y][x]);
		greenList[i] = GetGValue(colour[i][y][x]);
		blueList[i] = GetBValue(colour[i][y][x]);
	}








	// I have NOT been able to find a faster way to do mode, so currently this is the same as basic_fast_mode








	int redMode = *redList;
	int greenMode = *greenList;
	int blueMode = *blueList;

	int redMax = redList[0], redMin = redList[0];
	int greenMax = greenList[0], greenMin = greenList[0];
	int blueMax = blueList[0], blueMin = blueList[0];
	for (int i = 1; i < num; i++) {
		redMax = max(redMax, redList[i]);
		greenMax = max(greenMax, greenList[i]);
		blueMax = max(blueMax, blueList[i]);

		redMin = min(redMin, redList[i]);
		greenMin = min(greenMin, greenList[i]);
		blueMin = min(blueMin, blueList[i]);
	}

	if (redMax <= redMin + BASIC_FAST_MODE_RANGE && greenMax <= greenMin + BASIC_FAST_MODE_RANGE && blueMax <= blueMin + BASIC_FAST_MODE_RANGE) {
#ifdef SHR_DIV
		redMode = quick_div(redMin + redMax);
		greenMode = quick_div(greenMin + greenMax);
		blueMode = quick_div(blueMin + blueMax);
#else // SHR_DIV
		redMode = (redMin + redMax) / 2;
		greenMode = (greenMin + greenMax) / 2;
		blueMode = (blueMin + blueMax) / 2;
#endif // SHR_DIV
	}
	else {
		std::sort(redList, redList + num, std::greater<int>());
		std::sort(greenList, greenList + num, std::greater<int>());
		std::sort(blueList, blueList + num, std::greater<int>());

		int red_count = 1, max_redCount = 1;
		int green_count = 1, max_greenCount = 1;
		int blue_count = 1, max_blueCount = 1;
		for (int i = 1; i < num; i++) {
			if (redList[i] == redList[i - 1]) {
				red_count++;
			}
			else {
				if (red_count > max_redCount) {
					max_redCount = red_count;
					redMode = redList[i - 1];
				}
				red_count = 1;
			}

			if (greenList[i] == greenList[i - 1]) {
				green_count++;
			}
			else {
				if (green_count > max_greenCount) {
					max_greenCount = green_count;
					greenMode = greenList[i - 1];
				}
				green_count = 1;
			}

			if (blueList[i] == blueList[i - 1]) {
				blue_count++;
			}
			else {
				if (blue_count > max_blueCount) {
					max_blueCount = blue_count;
					blueMode = blueList[i - 1];
				}
				blue_count = 1;
			}
		}

		if (red_count > max_redCount) {
			redMode = redList[num - 1];
		}
		if (blue_count > max_blueCount) {
			blueMode = blueList[num - 1];
		}
		if (green_count > max_greenCount) {
			greenMode = greenList[num - 1];
		}
	}

	COLORREF ret = RGB(redMode, greenMode, blueMode);

	free(redList);
	free(greenList);
	free(blueList);

#ifdef PRINT_NUM_MODES
	modes++;
#endif // PRINT_NUM_MODES

	return ret;
}

#else // VERY_FAST_MEDIAN_MODE
// Default: Define a mode function that just takes the true mode
COLORREF Image::mode(COLORREF*** colour, int x, int y, const int num) {
	// Allocate enough memory for three arrays of ints, with size num
	int* redList = static_cast<int*>(malloc(num * sizeof(int)));
	int* greenList = static_cast<int*>(malloc(num * sizeof(int)));
	int* blueList = static_cast<int*>(malloc(num * sizeof(int)));

	// Break on bad allowance (debug feature)
#ifdef CHECK_BAD_ALLOC
	if (redList == nullptr) {
		quick_exit(1);
	}
	if (greenList == nullptr) {
		quick_exit(1);
	}
	if (blueList == nullptr) {
		quick_exit(1);
	}
#endif // CHECK_BAD_ALLOC

	// Convert colour to RGB arrays
	for (int i = 0; i < num; i++) {
		redList[i] = GetRValue(colour[i][y][x]);
		greenList[i] = GetGValue(colour[i][y][x]);
		blueList[i] = GetBValue(colour[i][y][x]);
	}

	// Sort arrays
	std::sort(redList, redList + num, std::greater<int>());
	std::sort(greenList, greenList + num, std::greater<int>());
	std::sort(blueList, blueList + num, std::greater<int>());

	int redMode = *redList;
	int greenMode = *greenList;
	int blueMode = *blueList;

	// A bit complicated, but the rest of this just finds the mode of each of the arrays
	int red_count = 1, max_redCount = 1;
	int green_count = 1, max_greenCount = 1;
	int blue_count = 1, max_blueCount = 1;
	for (int i = 1; i < num; i++) {
		if (redList[i] == redList[i - 1]) {
			red_count++;
		}
		else {
			if (red_count > max_redCount) {
				max_redCount = red_count;
				redMode = redList[i - 1];
			}
			red_count = 1;
		}

		if (greenList[i] == greenList[i - 1]) {
			green_count++;
		}
		else {
			if (green_count > max_greenCount) {
				max_greenCount = green_count;
				greenMode = greenList[i - 1];
			}
			green_count = 1;
		}

		if (blueList[i] == blueList[i - 1]) {
			blue_count++;
		}
		else {
			if (blue_count > max_blueCount) {
				max_blueCount = blue_count;
				blueMode = blueList[i - 1];
			}
			blue_count = 1;
		}
	}

	if (red_count > max_redCount) {
		redMode = redList[num - 1];
	}
	if (blue_count > max_blueCount) {
		blueMode = blueList[num - 1];
	}
	if (green_count > max_greenCount) {
		greenMode = greenList[num - 1];
	}

	// Cast return value to a COLORREF
	COLORREF ret = RGB(redMode, greenMode, blueMode);

	// Free any memory allocated to arrays
	free(redList);
	free(greenList);
	free(blueList);

	// If mode printing is on, increase that index
#ifdef PRINT_NUM_MODES
	modes++;
#endif // PRINT_NUM_MODES

	return ret;
}
#endif // VERY_FAST_MEDIAN_MODE
#endif // BASIC_FAST_MEDIAN_MODE
#endif // NO_MODE

// Virtual function, so I'm not bothering to write anything in it
void Image::ActualClean(int&, int&) {
	//Virtual, so don't care
}

// This clean will allow you to tell what width/height the return from ActualClean is
void Image::Clean(int& width, int& height) {
	ActualClean(width, height);
}

// This clean doesn't, but is recommended if you don't want that data
void Image::Clean() {
	int width = -1, height = -1;
	ActualClean(width, height);
}

// Write/fetch functions for pixel printing
#ifdef PRINT_PIXELS_PER_SECOND
void Image::SetReadTime(clock_t newTime) {
	readTime = newTime;
}

void Image::SetPixels(int newPixels) {
	m_pixels = newPixels;
}

clock_t Image::GetReadTime() {
	return readTime;
}

int Image::GetPixels() {
	return m_pixels;
}
#endif // PRINT_PIXELS_PER_SECOND

// Write/fetch functions for mean printing
#ifdef PRINT_NUM_MEANS
void Image::SetMeanTime(clock_t newTime) {
	meanTime = newTime;
}

void Image::SetMeans(int newMeans) {
	means = newMeans;
}

clock_t Image::GetMeanTime() {
	return meanTime;
}

int Image::GetMeans() {
	return means;
}
#endif // PRINT_NUM_MEANS

// Write/fetch functions for median printing
#ifdef PRINT_NUM_MEDIANS
void Image::SetMedianTime(clock_t newTime) {
	medianTime = newTime;
}

void Image::SetMedians(int newMedians) {
	medians = newMedians;
}

clock_t Image::GetMedianTime() {
	return medianTime;
}

int Image::GetMedians() {
	return medians;
}
#endif // PRINT_NUM_MEDIANS

// Write/fetch functions for mode printing
#ifdef PRINT_NUM_MODES
void Image::SetModeTime(clock_t newTime) {
	modeTime = newTime;
}

void Image::SetModes(int newModes) {
	modes = newModes;
}

clock_t Image::GetModeTime() {
	return modeTime;
}

int Image::GetModes() {
	return modes;
}
#endif // PRINT_NUM_MODES

// Path fetching function
std::string Image::GetPath() {
	return path;
}

// Output path return functions (if enabled)
#ifndef NO_MEAN
std::string Image::GetMeanOutput() {
	return meanOutput;
}
#endif // NO_MEAN

#ifndef NO_MEDIAN
std::string Image::GetMedianOutput() {
	return medianOutput;
}
#endif // NO_MEDIAN

#ifndef NO_MODE
std::string Image::GetModeOutput() {
	return modeOutput;
}
#endif // NO_MODE

// Saves BmpHeader onto file, don't worry about this
void BitmapImage::BmpHeader::save_on_file(std::ofstream& fout) {
	fout.write(this->bitmapSignatureBytes, 2);
	fout.write((char*)&this->sizeOfBitmapFile, sizeof(uint32_t));
	fout.write((char*)&this->reservedBytes, sizeof(uint32_t));
	fout.write((char*)&this->pixelDataOffset, sizeof(uint32_t));
}

// Saves BmpInfoHeader onto file, don't worry about this
void BitmapImage::BmpInfoHeader::save_on_file(std::ofstream& fout) {
	fout.write((char*)&this->sizeOfThisHeader, sizeof(uint32_t));
	fout.write((char*)&this->width, sizeof(int32_t));
	fout.write((char*)&this->height, sizeof(int32_t));
	fout.write((char*)&this->numberOfColorPlanes, sizeof(uint16_t));
	fout.write((char*)&this->colorDepth, sizeof(uint16_t));
	fout.write((char*)&this->compressionMethod, sizeof(uint32_t));
	fout.write((char*)&this->rawBitmapDataSize, sizeof(uint32_t));
	fout.write((char*)&this->horizontalResolution, sizeof(int32_t));
	fout.write((char*)&this->verticalResolution, sizeof(int32_t));
	fout.write((char*)&this->colorTableEntries, sizeof(uint32_t));
	fout.write((char*)&this->importantColors, sizeof(uint32_t));
}

// Reads in a bitmap image and converts it into a COLORREF** 2-D array object
COLORREF** BitmapImage::ReadImage(WCHAR* filename, int& width, int& height) {
	// Initialize a HBITMAP and HDC to load the image. WinAPI/GDI is a bit weird about this, but we have to init an HDC even if it's null and we're not using it
	HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	HDC hdc = CreateCompatibleDC(NULL);
	// We also have to do this, even though it doesn't make sense
	(HBITMAP)SelectObject(hdc, hBitmap);

	// Convert HBITMAP to BITMAP, which is more useful
	BITMAP bmp;
	GetObject(hBitmap, sizeof(bmp), &bmp);

	// Write some data into the bitmap header
	BITMAPINFO info;
	info.bmiHeader.biSize = sizeof(info.bmiHeader);
	info.bmiHeader.biWidth = bmp.bmWidth;
	info.bmiHeader.biHeight = bmp.bmHeight;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 24;
	info.bmiHeader.biCompression = BI_RGB;

	// Allocate enough memory for an array of COLORREF* objects with size bmp.bmHeight
	COLORREF** data = static_cast<COLORREF**>(malloc(bmp.bmHeight * sizeof(COLORREF*)));

	// Don't worry about this
	size_t pixelSize = info.bmiHeader.biBitCount / 8;
	size_t scanlineSize = (pixelSize * info.bmiHeader.biWidth + 3) & ~3;
	size_t bitmapSize = bmp.bmHeight * scanlineSize;

	// Load pixels into a pixels list
	std::vector<BYTE> pixels(bitmapSize);
	GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, &pixels[0], &info, DIB_RGB_COLORS);

	// If pixel printing is on, save current time
#ifdef PRINT_PIXELS_PER_SECOND
	SetReadTime(clock());
#endif // PRINT_PIXELS_PER_SECOND

	// The colour array is indexed by height FIRST, then width (it is very backwards, but every call is data[y][x]
	for (LONG y = 0; y < bmp.bmHeight; y++) {
		// Allocate memory for an array of COLORREF with size bmp.bmWidth
		COLORREF* miniData = static_cast<COLORREF*>(malloc(bmp.bmWidth * sizeof(COLORREF)));

		// Index through width and set the temp array[x] to the pixel at x, y
		for (LONG x = 0; x < bmp.bmWidth; x++) {
			size_t pixelOffset = y * scanlineSize + x * pixelSize;
			COLORREF color = RGB(
				pixels[pixelOffset + 2],
				pixels[pixelOffset + 1],
				pixels[pixelOffset + 0]);
			// Set temp array[x] to pixel at x, y
			miniData[x] = color;

			// If pixel print is on, increase that index by w
#ifdef PRINT_PIXELS_PER_SECOND
			SetPixels(GetPixels() + 1);
#endif // PRINT_PIXELS_PER_SECOND
		}
		// Set data[y] to point to our temp array
		// Note that I'm not freeing the temp array as we're still pointing to it in the main array
		// Pointers are basically memory addresses that we store and can use to access data without having to keep re-initializing/moving it
		// Data is an array of pointers, which is why it is COLORREF** instead of just COLORREF*
		data[y] = miniData;
	}
	
	// If pixel print is on, set clock to the time it took to do this
#ifdef PRINT_PIXELS_PER_SECOND
	SetReadTime(clock() - GetReadTime());
#endif // PRINT_PIXELS_PER_SECOND

	// Set the width/height variables that we are passing back to the bmp width/height
	// int& means that any edits we do to the input variable are also applied to the one passed in, as we're just passing in a pointer with int&
	if (width == -1 && height == -1) {
		width = bmp.bmWidth;
		height = bmp.bmHeight;
	}
	else {
		width = min(width, bmp.bmWidth);
		height = min(height, bmp.bmHeight);
	}

	// Again, I'm not freeing data as we're using it later and if I free it, then the pointer won't tell us anything
	return data;
}

// This is the actual clean function for BitmapImage child class ONLY
void BitmapImage::ActualClean(int& width, int& height) {
	COLORREF*** images;
	int num_images;

	// If this isn't a video, read images to get the images array
	if (optional_picture == nullptr) {
		// std::vector<std::wstring> is just a c++-style list of wstrings. I'm using c++-style list here as I don't know how much memory I should allocate to validPaths
		// Also c++-style lists don't need to be freed!
		std::vector<std::wstring> validPaths;

		// This is why this is a C++ 20 program. std::filesystem is new in this edition, and can be used to fetch all files in directory
		for (const auto& entry : std::filesystem::directory_iterator(GetPath())) {
			// Check if it ends in ".bmp", if it does, and it isn't one of the output files, then add it to the validPaths list
			std::string substring = entry.path().string().substr(strlen(entry.path().string().c_str()) - 4), strlen(entry.path().string().c_str());
			if (substring == ".bmp") {
				// All of this preprocessor garbage basically just allows you to toggle on/off mean, median, and mode
#ifndef NO_MEAN
				if (entry.path().string() != GetMeanOutput()) {
#ifndef NO_MEDIAN
					if (entry.path().string() != GetMedianOutput()) {
#ifndef NO_MODE
						if (entry.path().string() != GetModeOutput()) {
							std::wstring temp(entry.path().string().size(), L' ');
							temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
							validPaths.push_back(temp);
						}
#else // NO_MODE
						std::wstring temp(entry.path().string().size(), L' ');
						temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
						validPaths.push_back(temp);
#endif // NO_MODE
					}
#else // NO_MEDIAN
#ifndef NO_MODE
					if (entry.path().string() != GetModeOutput()) {
						std::wstring temp(entry.path().string().size(), L' ');
						temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
						validPaths.push_back(temp);
					}
#else // NO_MODE
					std::wstring temp(entry.path().string().size(), L' ');
					temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
					validPaths.push_back(temp);
#endif // NO_MODE
#endif // NO_MEDIAN
				}
#else // NO_MEAN
#ifndef NO_MEDIAN
				if (entry.path().string() != GetMedianOutput()) {
#ifndef NO_MODE
					if (entry.path().string() != GetModeOutput()) {
						std::wstring temp(entry.path().string().size(), L' ');
						temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
						validPaths.push_back(temp);
					}
#else // NO_MODE
					std::wstring temp(entry.path().string().size(), L' ');
					temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
					validPaths.push_back(temp);
#endif // NO_MODE
				}
#else // NO_MEDIAN
#ifndef NO_MODE
				if (entry.path().string() != GetModeOutput()) {
					std::wstring temp(entry.path().string().size(), L' ');
					temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
					validPaths.push_back(temp);
				}
#else // NO_MODE
				quick_exit(1);
#endif // NO_MODE
#endif // NO_MEDIAN
#endif // NO_MEAN
			}
		}

#ifdef PRINT_TIME_AFTER_FILESYSTEM
		std::cout << clock() << std::endl;
#endif // PRINT_TIME_AFTER_FILESYSTEM

		num_images = (int)validPaths.size();

		// Allocate enough memory for an array of COLORREF** (pointers to arrays of COLORREF) with size num_images
		// calloc is basically just malloc but you can pass in a non-const size
		images = static_cast<COLORREF***>(calloc(num_images, sizeof(COLORREF**)));
		// If CHECK_BAD_ALLOC is on, break on bad allowance (debug option)
#ifdef CHECK_BAD_ALLOC
		if (images == nullptr) {
			quick_exit(1);
		}
#endif // CHECK_BAD_ALLOC

		// Initialize images[i] to the COLORREF** that is the data of the file at validPaths[i]
#ifdef TRY_MULTITHREAD_LOAD
		auto imageReadThread = [&](int j) {
			// It splits up total num_images into 4 chunks
#ifdef SHR_DIV
			for (int i = quick_div(quick_div(num_images * (j - 1))); i < quick_div(quick_div(num_images * j)); i++) {
#else // SHR_DIV
			for (int i = num_images * (j - 1) / 4; i < num_images * j / 4; i++) {
#endif // SHR_DIV
				// Store ReadImage in srcPtr
				images[i] = ReadImage((WCHAR*)validPaths[i].c_str(), width, height);
			}
			};

		// Init threads for reading the image
		std::thread imageReadThread1(imageReadThread, 1);
		std::thread imageReadThread2(imageReadThread, 2);
		std::thread imageReadThread3(imageReadThread, 3);
		std::thread imageReadThread4(imageReadThread, 4);

		// Rejoin image reading threads
		imageReadThread1.join();
		imageReadThread2.join();
		imageReadThread3.join();
		imageReadThread4.join();
#else // TRY_MULTITHREAD_LOAD
		for (int i = 0; i < num_images; i++) {
			images[i] = ReadImage((WCHAR*)validPaths[i].c_str(), width, height);
		}
#endif // TRY_MULTITHREAD_LOAD
	}
	else {
		images = optional_picture;
		num_images = optional_num_pics;
	}

	// Set some bmpHeader things, don't worry about it
#ifdef WRITE_HEADER_SEPERATELY
	// If we're writing header seperately, use a lambda function and a thread to start that up
	auto headerWrite = [&]() {
#endif // WRITE_HEADER_SEPERATELY
		bmpInfoHeader.width = width;
		bmpInfoHeader.height = height;

		bmpHeader.sizeOfBitmapFile = width * height * bmpInfoHeader.colorDepth;

#ifdef WRITE_HEADER_SEPERATELY
		// Write header to file
		std::ofstream foutMean(GetMeanOutput(), std::ios::binary);
		std::ofstream foutMedian(GetMedianOutput(), std::ios::binary);
		std::ofstream foutMode(GetModeOutput(), std::ios::binary);
		bmpHeader.save_on_file(foutMean);
		bmpInfoHeader.save_on_file(foutMean);
		bmpHeader.save_on_file(foutMedian);
		bmpInfoHeader.save_on_file(foutMedian);
		bmpHeader.save_on_file(foutMode);
		bmpInfoHeader.save_on_file(foutMode);
		foutMean.close();
		foutMedian.close();
		foutMode.close();
	};

	std::thread headerWriteThread(headerWrite);
#endif // WRITE_HEADER_SEPERATELY

	// Allocate enough memory for mean/median/mode lists IF those are toggled on
#ifndef NO_MEAN
	COLORREF** imageDataMean = static_cast<COLORREF**>(malloc(height * sizeof(COLORREF*)));
#endif // NO_MEAN
#ifndef NO_MEDIAN
	COLORREF** imageDataMedian = static_cast<COLORREF**>(malloc(height * sizeof(COLORREF*)));
#endif // NO_MEDIAN
#ifndef NO_MODE
	COLORREF** imageDataMode = static_cast<COLORREF**>(malloc(height * sizeof(COLORREF*)));
#endif // NO_MODE
	// Again, break on bad alloc if CHECK_BAD_ALLOC is on
#ifdef CHECK_BAD_ALLOC
	if (imageDataMean == nullptr) {
		quick_exit(1);
	}
	if (imageDataMedian == nullptr) {
		quick_exit(1);
	}
	if (imageDataMode == nullptr) {
		quick_exit(1);
	}
#endif // CHECK_BAD_ALLOC

	// If we're doing mean, define a lambda function that will take the mean of images and put it into the imageDataMean array we defined earlier
	// Lambda functions are weird, but multithreading is a LOT easier with them, so I'm using them
#ifndef NO_MEAN
	// [&]() means that we're capturing every local variable in the lambda function and can reference them
	auto doMean = [&]() {
#ifdef PRINT_NUM_MEANS
		SetMeanTime(clock());
#endif // PRINT_NUM_MEANS

		// Index through height first, as everything is [height][width]
		for (int y = 0; y < height; y++) {
			// Allocate memory for an array of mean at y, x, for x in width
			COLORREF* tempDataMean = static_cast<COLORREF*>(malloc(width * sizeof(COLORREF)));
			// Break on bad alloc if that's enabled
#ifdef CHECK_BAD_ALLOC
			if (tempDataMean == nullptr) {
				quick_exit(1);
			}
#endif // CHECK_BAD_ALLOC
			// Initialize tempDataMean
			for (int x = 0; x < width; x++) {
				tempDataMean[x] = mean(images, x, y, num_images);
			}
			// Make imageDataMean[y] point to tempDataMean. I'm not freeing it again as we're still using it later
			imageDataMean[y] = tempDataMean;
		}

		// If we're counting mean time, set that to the time it took to do this
#ifdef PRINT_NUM_MEANS
		SetMeanTime(clock() - GetMeanTime());
#endif // PRINT_NUM_MEANS
	};
#endif // NO_MEAN

	// Another lambda function, this time to calculate median
#ifndef NO_MEDIAN
	auto doMedian = [&]() {
#ifdef PRINT_NUM_MEDIANS
		SetMedianTime(clock());
#endif // PRINT_NUM_MEDIANS

		// Same as with mean, index through height then width
		for (int y = 0; y < height; y++) {
			// Allocate memory for tempDataMedian (width-sized array of COLORREF)
			COLORREF* tempDataMedian = static_cast<COLORREF*>(malloc(width * sizeof(COLORREF)));
			// Break on bad alloc if that's enabled
#ifdef CHECK_BAD_ALLOC
			if (tempDataMedian == nullptr) {
				quick_exit(1);
			}
#endif // CHECK_BAD_ALLOC
			for (int x = 0; x < width; x++) {
				// Set tempDataMedian[x] to whichever median option is currently enabled. Note that basic_fast_median will override very_fast_median if both are enabled
#ifdef BASIC_FAST_MEDIAN_MODE
				tempDataMedian[x] = basic_fast_median(images, x, y, num_images);
#else // BASIC_FAST_MEDIAN_MODE
#ifdef VERY_FAST_MEDIAN_MODE
				tempDataMedian[x] = very_fast_median(images, x, y, num_images);
#else // VERY_FAST_MEDIAN_MODE
				tempDataMedian[x] = median(images, x, y, num_images);
#endif // VERY_FAST_MEDIAN_MODE
#endif // BASIC_FAST_MEDIAN_MODE
			}
			// Set imageDataMedian[y] to point to tempDataMedian. Again, not freeing as I'm still accessing that memory later
			imageDataMedian[y] = tempDataMedian;
		}

		// If we're printing mean time, set that to the time it took to run this function
#ifdef PRINT_NUM_MEDIANS
		SetMedianTime(clock() - GetMedianTime());
#endif // PRINT_NUM_MEDIANS
	};
#endif // NO_MEDIAN

	// Another lambda function, this time for mode
#ifndef NO_MODE
	auto doMode = [&]() {
#ifdef PRINT_NUM_MODES
		SetModeTime(clock());
#endif // PRINT_NUM_MODES

		// Do the same thing where we index height, then width
		for (int y = 0; y < height; y++) {
			// Allocate memory for tempDataMode (width-sized array of COLORREF)
			COLORREF* tempDataMode = static_cast<COLORREF*>(malloc(width * sizeof(COLORREF)));
			// If CHECK_BAD_ALLOC is on, break on bad alloc
#ifdef CHECK_BAD_ALLOC
			if (tempDataMode == nullptr) {
				quick_exit(1);
			}
#endif // CHECK_BAD_ALLOC
			for (int x = 0; x < width; x++) {
				// Just like with median, run the mode version that's enabled. Still, VERY_FAST_MEDIAN_MODE trumps BASIC_FAST_MEDIAN_MODE
#ifdef BASIC_FAST_MEDIAN_MODE
				tempDataMode[x] = basic_fast_mode(images, x, y, num_images);
#else // BASIC_FAST_MEDIAN_MODE
#ifdef VERY_FAST_MEDIAN_MODE
				tempDataMode[x] = very_fast_mode(images, x, y, num_images);
#else // VERY_FAST_MEDIAN_MODE
				tempDataMode[x] = mode(images, x, y, num_images);
#endif // VERY_FAST_MEDIAN_MODE
#endif // BASIC_FAST_MEDIAN_MODE
			}
			// Set imageDataMode[y] to point to tempDataMode. Again not freeing as I'm accessing this data later
			imageDataMode[y] = tempDataMode;
		}

		// If we're printing mode time, set this to the time it took to run this function
#ifdef PRINT_NUM_MODES
		SetModeTime(clock() - GetModeTime());
#endif // PRINT_NUM_MODES
	};
#endif // NO_MODE

	// Create threads for mean/median/mode. Threads will run in parallel and will run their respective lambda functions.
	// They run either until they are joined (when the rest of the program waits for them to finish) or they finish by themselves.
#ifndef NO_MEAN
	std::thread meanThread(doMean);
#endif // NO_MEAN
#ifndef NO_MEDIAN
	std::thread medianThread(doMedian);
#endif // NO_MEDIAN
#ifndef NO_MODE
	std::thread modeThread(doMode);
#endif // NO_MODE

#ifdef PRINT_TIME_BEFORE_WRITING
	std::cout << "Finished processing data at: " << clock() << std::endl;
#endif // PRINT_TIME_BEFORE_WRITING
#ifdef WRITE_HEADER_SEPERATELY
	// Rejoin header write thread
	headerWriteThread.join();
#endif // WRITE_HEADER_SEPERATELY

	// We have another set of lambda functions, but these ones are for writing our mean/median/mode output to the files they're going to
#ifndef NO_MEAN
	auto meanWrite = [&]() {
		// std::ofstream just lets us open a file and write to it
#ifdef WRITE_HEADER_SEPERATELY
		std::ofstream foutMean(GetMeanOutput(), std::ios::binary | std::ios::app);
#else // WRITE_HEADER_SEPERATELY
		std::ofstream foutMean(GetMeanOutput(), std::ios::binary);
#endif // WRITE_HEADER_SEPERATELY

#ifndef WRITE_HEADER_SEPERATELY
		// Write the header first
		bmpHeader.save_on_file(foutMean);
		bmpInfoHeader.save_on_file(foutMean);
#endif // WRITE_HEADER_SEPERATELY

		// While we index through the file, create a Pixel object and write that to the file for each y,x
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				// Save each pixel y,x to the file
#ifndef NO_PIXEL_STRUCT
				Pixel pixel{ GetBValue(imageDataMean[y][x]), GetGValue(imageDataMean[y][x]), GetRValue(imageDataMean[y][x]) };
				pixel.save_on_file(foutMean);
#else // NO_PIXEL_STRUCT
				uint8_t b = GetBValue(imageDataMean[y][x]);
				uint8_t g = GetGValue(imageDataMean[y][x]);
				uint8_t r = GetRValue(imageDataMean[y][x]);
				foutMean.write((char*)&b, sizeof(uint8_t));
				foutMean.write((char*)&g, sizeof(uint8_t));
				foutMean.write((char*)&r, sizeof(uint8_t));
#endif // NO_PIXEL_STRUCT
			}
			// Now we're done with imageDataMean[y], so we can free it
			free(imageDataMean[y]);
		}
		// We have to close the file before destroying foutMean
		foutMean.close();
	};
#endif // NO_MEAN

#ifndef NO_MEDIAN
	auto medianWrite = [&]() {
		// Open median output file
#ifdef WRITE_HEADER_SEPERATELY
		std::ofstream foutMedian(GetMedianOutput(), std::ios::binary | std::ios::app);
#else // WRITE_HEADER_SEPERATELY
		std::ofstream foutMedian(GetMedianOutput(), std::ios::binary);
#endif // WRITE_HEADER_SEPERATELY

#ifndef WRITE_HEADER_SEPERATELY
		// Write headers to the file
		bmpHeader.save_on_file(foutMedian);
		bmpInfoHeader.save_on_file(foutMedian);
#endif

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				// Save each pixel y,x to the file
#ifndef NO_PIXEL_STRUCT
				Pixel pixel2{ GetBValue(imageDataMedian[y][x]), GetGValue(imageDataMedian[y][x]), GetRValue(imageDataMedian[y][x]) };
				pixel2.save_on_file(foutMedian);
#else // NO_PIXEL_STRUCT
				uint8_t b = GetBValue(imageDataMedian[y][x]);
				uint8_t g = GetGValue(imageDataMedian[y][x]);
				uint8_t r = GetRValue(imageDataMedian[y][x]);
				foutMedian.write((char*)&b, sizeof(uint8_t));
				foutMedian.write((char*)&g, sizeof(uint8_t));
				foutMedian.write((char*)&r, sizeof(uint8_t));
#endif // NO_PIXEL_STRUCT
			}
			// We're done with imageDataMedian[y] now, so we can free it
			free(imageDataMedian[y]);
		}
		// Close median output file
		foutMedian.close();
	};
#endif // NO_MEDIAN

#ifndef NO_MODE
	auto modeWrite = [&]() {
		// Open mode output file
#ifdef WRITE_HEADER_SEPERATELY
		std::ofstream foutMode(GetModeOutput(), std::ios::binary | std::ios::app);
#else // WRITE_HEADER_SEPERATELY
		std::ofstream foutMode(GetModeOutput(), std::ios::binary);
#endif // WRITE_HEADER_SEPERATELY

#ifndef WRITE_HEADER_SEPERATELY
		// Save headers on file
		bmpHeader.save_on_file(foutMode);
		bmpInfoHeader.save_on_file(foutMode);
#endif

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				// Save each pixel to the file
#ifndef NO_PIXEL_STRUCT
				Pixel pixel3{ GetBValue(imageDataMode[y][x]), GetGValue(imageDataMode[y][x]), GetRValue(imageDataMode[y][x]) };
				pixel3.save_on_file(foutMode);
#else // NO_PIXEL_STRUCT
				uint8_t b = GetBValue(imageDataMode[y][x]);
				uint8_t g = GetGValue(imageDataMode[y][x]);
				uint8_t r = GetRValue(imageDataMode[y][x]);
				foutMode.write((char*)&b, sizeof(uint8_t));
				foutMode.write((char*)&g, sizeof(uint8_t));
				foutMode.write((char*)&r, sizeof(uint8_t));
#endif // NO_PIXEL_STRUCT
			}
			// We're done with imageDataMode[y], so we can free it
			free(imageDataMode[y]);
		}
		// Close file
		foutMode.close();
	};
#endif // NO_MODE

	// We will wait until the mean/median/mode threads join, then start threads that write them to their respective files
	// We have to wait or we will get a memory access error as I haven't used mutex or atomic
#ifndef NO_MEAN
	meanThread.join();
	std::thread meanWriteThread(meanWrite);
#endif // NO_MEAN
#ifndef NO_MEDIAN
	medianThread.join();
	std::thread medianWriteThread(medianWrite);
#endif // NO_MEDIAN
#ifndef NO_MODE
	modeThread.join();
	std::thread modeWriteThread(modeWrite);
#endif // NO_MODE

	// Now we're done with images, so we can free it
	for (int i = 0; i < num_images; i++) {
		// As images is a 3-D array, we have to free images[i][y], then images[i], then images
		// images = {{{...},{...},{...},{...},...},{{...},{...},{...},...},...}, so we have to free each of these sub-arrays or we will have a memory leak
		for (int y = 0; y < height; y++) {
			free(images[i][y]);
		}
		free(images[i]);
	}
	free(images);

	// Join up the write threads, then free the imageDataMean/Median/Mode array as we're done with it
	// Even though it is a 2-D array, we've freed all of the subarrays in the respective write lambda functions, so we don't have to do that again
#ifndef NO_MEAN
	meanWriteThread.join();
	free(imageDataMean);
#endif // NO_MEAN
#ifndef NO_MEDIAN
	medianWriteThread.join();
	free(imageDataMedian);
#endif // NO_MEDIAN
#ifndef NO_MODE
	modeWriteThread.join();
	free(imageDataMode);
#endif // NO_MODE

	// If any prints are enabled, do those. Pretty print is just another way of printing it
#ifdef PRINT_PIXELS_PER_SECOND
#ifdef PRETTY_PRINT
	float divVal = (float)GetReadTime() / 1000.0f;
	std::cout << "Reading pixels at a rate of " << (float)GetPixels() / divVal << " pixels per second" << std::endl;
#else // PRETTY_PRINT
	std::cout << GetPixels() << " pixels read in " << GetReadTime() << " milliseconds" << std::endl;
#endif // PRETTY_PRINT
#endif // PRINT_PIXELS_PER_SECOND
#ifdef PRINT_NUM_MEANS
#ifdef PRETTY_PRINT
	divVal = (float)GetMeanTime() / 1000.0f;
	std::cout << "Calculating means at a rate of " << (float)GetMeans() / divVal << " means per second" << std::endl;
#else // PRETTY_PRINT
	std::cout << GetMeans() << " means calculated in " << GetMeanTime() << " milliseconds" << std::endl;
#endif // PRETTY_PRINT
#endif // PRINT_NUM_MEANS
#ifdef PRINT_NUM_MEDIANS
#ifdef PRETTY_PRINT
	divVal = (float)GetMedianTime() / 1000.0f;
	std::cout << "Calculating medians at a rate of " << (float)GetMedians() / divVal << " medians per second" << std::endl;
#else // PRETTY_PRINT
	std::cout << GetMedians() << " medians calculated in " << GetMedianTime() << " milliseconds" << std::endl;
#endif // PRETTY_PRINT
#endif // PRINT_NUM_MEDIANS
#ifdef PRINT_NUM_MODES
#ifdef PRETTY_PRINT
	divVal = (float)GetModeTime() / 1000.0f;
	std::cout << "Calculating modes at a rate of " << (float)GetModes() / divVal << " modes per second" << std::endl;
#else // PRETTY_PRINT
	std::cout << GetModes() << " modes calculated in " << GetModeTime() << " milliseconds" << std::endl;
#endif // PRETTY_PRINT
#endif // PRINT_NUM_MODES
}

// This is JPEGImage's ReadImage function. It is only called by JPEGImage objects, NOT BitmapImage
COLORREF** JPEGImage::ReadImage(WCHAR* filename, int& width, int& height) {
#ifdef PRINT_PIXELS_PER_SECOND
	SetReadTime(clock());
#endif // PRINT_PIXELS_PER_SECOND
	int Width, Height, channels;
	// Convert filename (a wchar_t* object) into std::string
	std::wstring name = std::wstring(filename);
	std::string filePath = std::string(name.begin(), name.end());
	// Load the data at filename into an array. This array works weird
	// Channels is just 3 as we only care about RGB
	unsigned char* test = stbi_load(filePath.c_str(), &Width, &Height, &channels, 3);

	// Allocate data for an array of COLORREF* with size Width*Height
	// Using calloc as Width*Height isn't constant
	COLORREF** data = static_cast<COLORREF**>(calloc(Width * Height, sizeof(COLORREF*)));
	for (int i = 0; i < Height; i++) {
		// Allocate a subarray for the x's at height i
		// Using calloc since Width is not const
		COLORREF* tempData = static_cast<COLORREF*>(calloc(Width, sizeof(COLORREF)));
		for (int j = 0; j < Width; j++) {
			// stbi_load stores data really weirdly, basically as r(0,0), g(0,0), b(0,0), r(1,0), g(1,0), b(1,0), ...
			// *(thing) = the value stored at memory address thing
			// We can add memory addresses, which is useful as arrays are stored sequentially
			// Also, *(thing) ~~ array thing is part of[thing].
			uint8_t r = *(test + 3 * j + 3 * i * Width);
			uint8_t g = *(test + 3 * j + 3 * i * Width + 1);
			uint8_t b = *(test + 3 * j + 3 * i * Width + 2);

			// Cast to a COLORREF, then set tempData[j] to this
			COLORREF cur = RGB((int)r, (int)g, (int)b);
			tempData[j] = cur;

			// If we're printing pixels, increase that index by 1
#ifdef PRINT_PIXELS_PER_SECOND
			SetPixels(GetPixels() + 1);
#endif // PRINT_PIXELS_PER_SECOND
		}
		// Set data[i] to temp data. Not freeing as we're using tempData's data later
		data[i] = tempData;
	}

	// If we're printing pixels, set the time to the time it took to do this
#ifdef PRINT_PIXELS_PER_SECOND
	SetReadTime(clock() - GetReadTime());
#endif // PRINT_PIXELS_PER_SECOND

	// "Return" width and height by setting the input pointed variables to Width and Height
	if (width == -1 && height == -1) {
		width = Width;
		height = Height;
	}
	// We only want the smallest width/height, so return that if there's a problem
	else {
		width = min(width, Width);
		height = min(height, Height);
	}

	// Don't free data as we're using it later
	return data;
}

// JPEGImage's ActualClean. The only difference between this and BitmapImage's is that it loads all of the .jpg files instead of the .bmp files.
// I was going to make it output to .jpg, but I couldn't figure out how to do that
void JPEGImage::ActualClean(int& width, int& height) {
	std::vector<std::wstring> validPaths;

	for (const auto& entry : std::filesystem::directory_iterator(GetPath())) {
		std::string substring = entry.path().string().substr(strlen(entry.path().string().c_str()) - 4), strlen(entry.path().string().c_str());
		if (substring == ".jpg") {
#ifndef NO_MEAN
			if (entry.path().string() != GetMeanOutput()) {
#ifndef NO_MEDIAN
				if (entry.path().string() != GetMedianOutput()) {
#ifndef NO_MODE
					if (entry.path().string() != GetModeOutput()) {
						std::wstring temp(entry.path().string().size(), L' ');
						temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
						validPaths.push_back(temp);
					}
#else // NO_MODE
					std::wstring temp(entry.path().string().size(), L' ');
					temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
					validPaths.push_back(temp);
#endif // NO_MODE
				}
#else // NO_MEDIAN
#ifndef NO_MODE
				if (entry.path().string() != GetModeOutput()) {
					std::wstring temp(entry.path().string().size(), L' ');
					temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
					validPaths.push_back(temp);
				}
#else // NO_MODE
				std::wstring temp(entry.path().string().size(), L' ');
				temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
				validPaths.push_back(temp);
#endif // NO_MODE
#endif // NO_MEDIAN
			}
#else // NO_MEAN
#ifndef NO_MEDIAN
			if (entry.path().string() != GetMedianOutput()) {
#ifndef NO_MODE
				if (entry.path().string() != GetModeOutput()) {
					std::wstring temp(entry.path().string().size(), L' ');
					temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
					validPaths.push_back(temp);
				}
#else // NO_MODE
				std::wstring temp(entry.path().string().size(), L' ');
				temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
				validPaths.push_back(temp);
#endif // NO_MODE
			}
#else // NO_MEDIAN
#ifndef NO_MODE
			if (entry.path().string() != GetModeOutput()) {
				std::wstring temp(entry.path().string().size(), L' ');
				temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
				validPaths.push_back(temp);
			}
#else // NO_MODE
			quick_exit(1);
#endif // NO_MODE
#endif // NO_MEDIAN
#endif // NO_MEAN
		}
	}

#ifdef PRINT_TIME_AFTER_FILESYSTEM
	std::cout << clock() << std::endl;
#endif // PRINT_TIME_AFTER_FILESYSTEM

	int num_images = (int)validPaths.size();

	COLORREF*** images = static_cast<COLORREF***>(calloc(num_images, sizeof(COLORREF**)));
#ifdef CHECK_BAD_ALLOC
	if (images == nullptr) {
		quick_exit(1);
	}
#endif // CHECK_BAD_ALLOC

#ifdef TRY_MULTITHREAD_LOAD
	auto imageReadThread = [&](int j) {
		// It splits up total num_images into 4 chunks
#ifdef SHR_DIV
		for (int i = quick_div(quick_div(num_images * (j - 1))); i < quick_div(quick_div(num_images * j)); i++) {
#else // SHR_DIV
		for (int i = num_images * (j - 1) / 4; i < num_images * j / 4; i++) {
#endif // SHR_DIV
			// Store ReadImage in srcPtr
			images[i] = ReadImage((WCHAR*)validPaths[i].c_str(), width, height);
		}
		};

	// Init threads for reading the image
	std::thread imageReadThread1(imageReadThread, 1);
	std::thread imageReadThread2(imageReadThread, 2);
	std::thread imageReadThread3(imageReadThread, 3);
	std::thread imageReadThread4(imageReadThread, 4);

	// Rejoin image reading threads
	imageReadThread1.join();
	imageReadThread2.join();
	imageReadThread3.join();
	imageReadThread4.join();
#else // TRY_MULTITHREAD_LOAD
	for (int i = 0; i < num_images; i++) {
		images[i] = ReadImage((WCHAR*)validPaths[i].c_str(), width, height);
	}
#endif // TRY_MULTITHREAD_LOAD

	// Set some bmpHeader things, don't worry about it
#ifdef WRITE_HEADER_SEPERATELY
	// If we're writing header seperately, use a lambda function and a thread to start that up
	auto headerWrite = [&]() {
#endif // WRITE_HEADER_SEPERATELY
		bmpInfoHeader.width = width;
		bmpInfoHeader.height = height;

		bmpHeader.sizeOfBitmapFile = width * height * bmpInfoHeader.colorDepth;

#ifdef WRITE_HEADER_SEPERATELY
		// Write header to file
		std::ofstream foutMean(GetMeanOutput(), std::ios::binary);
		std::ofstream foutMedian(GetMedianOutput(), std::ios::binary);
		std::ofstream foutMode(GetModeOutput(), std::ios::binary);
		bmpHeader.save_on_file(foutMean);
		bmpInfoHeader.save_on_file(foutMean);
		bmpHeader.save_on_file(foutMedian);
		bmpInfoHeader.save_on_file(foutMedian);
		bmpHeader.save_on_file(foutMode);
		bmpInfoHeader.save_on_file(foutMode);
		foutMean.close();
		foutMedian.close();
		foutMode.close();
	};

	std::thread headerWriteThread(headerWrite);
#endif // WRITE_HEADER_SEPERATELY

#ifndef NO_MEAN
	COLORREF** imageDataMean = static_cast<COLORREF**>(malloc(height * sizeof(COLORREF*)));
#endif // NO_MEAN
#ifndef NO_MEDIAN
	COLORREF** imageDataMedian = static_cast<COLORREF**>(malloc(height * sizeof(COLORREF*)));
#endif // NO_MEDIAN
#ifndef NO_MODE
	COLORREF** imageDataMode = static_cast<COLORREF**>(malloc(height * sizeof(COLORREF*)));
#endif // NO_MODE
#ifdef CHECK_BAD_ALLOC
	if (imageDataMean == nullptr) {
		quick_exit(1);
	}
	if (imageDataMedian == nullptr) {
		quick_exit(1);
	}
	if (imageDataMode == nullptr) {
		quick_exit(1);
	}
#endif // CHECK_BAD_ALLOC

#ifndef NO_MEAN
	auto doMean = [&]() {
#ifdef PRINT_NUM_MEANS
		SetMeanTime(clock());
#endif // PRINT_NUM_MEANS

		for (int y = 0; y < height; y++) {
			COLORREF* tempDataMean = static_cast<COLORREF*>(malloc(width * sizeof(COLORREF)));
#ifdef CHECK_BAD_ALLOC
			if (tempDataMean == nullptr) {
				quick_exit(1);
			}
#endif // CHECK_BAD_ALLOC
			for (int x = 0; x < width; x++) {
				tempDataMean[x] = mean(images, x, y, num_images);
			}
			imageDataMean[y] = tempDataMean;
		}

#ifdef PRINT_NUM_MEANS
		SetMeanTime(clock() - GetMeanTime());
#endif // PRINT_NUM_MEANS
	};
#endif // NO_MEAN

#ifndef NO_MEDIAN
	auto doMedian = [&]() {
#ifdef PRINT_NUM_MEDIANS
		SetMedianTime(clock());
#endif // PRINT_NUM_MEDIANS

		for (int y = 0; y < height; y++) {
			COLORREF* tempDataMedian = static_cast<COLORREF*>(malloc(width * sizeof(COLORREF)));
#ifdef CHECK_BAD_ALLOC
			if (tempDataMedian == nullptr) {
				quick_exit(1);
			}
#endif // CHECK_BAD_ALLOC
			for (int x = 0; x < width; x++) {
#ifdef BASIC_FAST_MEDIAN_MODE
				tempDataMedian[x] = basic_fast_median(images, x, y, num_images);
#else // BASIC_FAST_MEDIAN_MODE
#ifdef VERY_FAST_MEDIAN_MODE
				tempDataMedian[x] = very_fast_median(images, x, y, num_images);
#else // VERY_FAST_MEDIAN_MODE
				tempDataMedian[x] = median(images, x, y, num_images);
#endif // VERY_FAST_MEDIAN_MODE
#endif // BASIC_FAST_MEDIAN_MODE
			}
			imageDataMedian[y] = tempDataMedian;
		}

#ifdef PRINT_NUM_MEDIANS
		SetMedianTime(clock() - GetMedianTime());
#endif // PRINT_NUM_MEDIANS
	};
#endif // NO_MEDIAN

#ifndef NO_MODE
	auto doMode = [&]() {
#ifdef PRINT_NUM_MODES
		SetModeTime(clock());
#endif // PRINT_NUM_MODES

		for (int y = 0; y < height; y++) {
			COLORREF* tempDataMode = static_cast<COLORREF*>(malloc(width * sizeof(COLORREF)));
#ifdef CHECK_BAD_ALLOC
			if (tempDataMode == nullptr) {
				quick_exit(1);
			}
#endif // CHECK_BAD_ALLOC
			for (int x = 0; x < width; x++) {
#ifdef BASIC_FAST_MEDIAN_MODE
				tempDataMode[x] = basic_fast_mode(images, x, y, num_images);
#else // BASIC_FAST_MEDIAN_MODE
#ifdef VERY_FAST_MEDIAN_MODE
				tempDataMode[x] = very_fast_mode(images, x, y, num_images);
#else // VERY_FAST_MEDIAN_MODE
				tempDataMode[x] = mode(images, x, y, num_images);
#endif // VERY_FAST_MEDIAN_MODE
#endif // BASIC_FAST_MEDIAN_MODE
			}
			imageDataMode[y] = tempDataMode;
		}

#ifdef PRINT_NUM_MODES
		SetModeTime(clock() - GetModeTime());
#endif // PRINT_NUM_MODES
	};
#endif // NO_MEDIAN

#ifndef NO_MEAN
	std::thread meanThread(doMean);
#endif // NO_MEAN
#ifndef NO_MEDIAN
	std::thread medianThread(doMedian);
#endif // NO_MEDIAN
#ifndef NO_MODE
	std::thread modeThread(doMode);
#endif // NO_MODE

#ifdef PRINT_TIME_BEFORE_WRITING
	std::cout << "Finished processing data at: " << clock() << std::endl;
#endif // PRINT_TIME_BEFORE_WRITING

#ifdef WRITE_HEADER_SEPERATELY
	// Rejoin header write thread
	headerWriteThread.join();
#endif // WRITE_HEADER_SEPERATELY

#ifndef NO_MEAN
	auto meanWrite = [&]() {
#ifdef WRITE_HEADER_SEPERATELY
		std::ofstream foutMean(GetMeanOutput(), std::ios::binary | std::ios::app);
#else // WRITE_HEADER_SEPERATELY
		std::ofstream foutMean(GetMeanOutput(), std::ios::binary);
#endif // WRITE_HEADER_SEPERATELY

#ifndef WRITE_HEADER_SEPERATELY
		bmpHeader.save_on_file(foutMean);
		bmpInfoHeader.save_on_file(foutMean);
#endif // WRITE_HEADER_SEPERATELY

		bmpHeader.save_on_file(foutMean);
		bmpInfoHeader.save_on_file(foutMean);

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
#ifndef NO_PIXEL_STRUCT
				Pixel pixel{ GetBValue(imageDataMean[y][x]), GetGValue(imageDataMean[y][x]), GetRValue(imageDataMean[y][x]) };
				pixel.save_on_file(foutMean);
#else // NO_PIXEL_STRUCT
				uint8_t b = GetBValue(imageDataMean[y][x]);
				uint8_t g = GetGValue(imageDataMean[y][x]);
				uint8_t r = GetRValue(imageDataMean[y][x]);
				foutMean.write((char*)&b, sizeof(uint8_t));
				foutMean.write((char*)&g, sizeof(uint8_t));
				foutMean.write((char*)&r, sizeof(uint8_t));
#endif // NO_PIXEL_STRUCT
			}
			free(imageDataMean[y]);
		}
		foutMean.close();
	};
#endif // NO_MEAN

#ifndef NO_MEDIAN
	auto medianWrite = [&]() {
#ifdef WRITE_HEADER_SEPERATELY
		std::ofstream foutMedian(GetMedianOutput(), std::ios::binary | std::ios::app);
#else // WRITE_HEADER_SEPERATELY
		std::ofstream foutMedian(GetMedianOutput(), std::ios::binary);
#endif // WRITE_HEADER_SEPERATELY

#ifndef WRITE_HEADER_SEPERATELY
		// Write headers to the file
		bmpHeader.save_on_file(foutMedian);
		bmpInfoHeader.save_on_file(foutMedian);
#endif

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
#ifndef NO_PIXEL_STRUCT
				Pixel pixel2{ GetBValue(imageDataMedian[y][x]), GetGValue(imageDataMedian[y][x]), GetRValue(imageDataMedian[y][x]) };
				pixel2.save_on_file(foutMedian);
#else // NO_PIXEL_STRUCT
				uint8_t b = GetBValue(imageDataMedian[y][x]);
				uint8_t g = GetGValue(imageDataMedian[y][x]);
				uint8_t r = GetRValue(imageDataMedian[y][x]);
				foutMedian.write((char*)&b, sizeof(uint8_t));
				foutMedian.write((char*)&g, sizeof(uint8_t));
				foutMedian.write((char*)&r, sizeof(uint8_t));
#endif // NO_PIXEL_STRUCT
			}
			free(imageDataMedian[y]);
		}
		foutMedian.close();
	};
#endif // NO_MEDIAN

#ifndef NO_MODE
	auto modeWrite = [&]() {
#ifdef WRITE_HEADER_SEPERATELY
		std::ofstream foutMode(GetModeOutput(), std::ios::binary | std::ios::app);
#else // WRITE_HEADER_SEPERATELY
		std::ofstream foutMode(GetModeOutput(), std::ios::binary);
#endif // WRITE_HEADER_SEPERATELY

#ifndef WRITE_HEADER_SEPERATELY
		// Save headers on file
		bmpHeader.save_on_file(foutMode);
		bmpInfoHeader.save_on_file(foutMode);
#endif

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
#ifndef NO_PIXEL_STRUCT
				Pixel pixel3{ GetBValue(imageDataMode[y][x]), GetGValue(imageDataMode[y][x]), GetRValue(imageDataMode[y][x]) };
				pixel3.save_on_file(foutMode);
#else // NO_PIXEL_STRUCT
				uint8_t b = GetBValue(imageDataMode[y][x]);
				uint8_t g = GetGValue(imageDataMode[y][x]);
				uint8_t r = GetRValue(imageDataMode[y][x]);
				foutMode.write((char*)&b, sizeof(uint8_t));
				foutMode.write((char*)&g, sizeof(uint8_t));
				foutMode.write((char*)&r, sizeof(uint8_t));
#endif // NO_PIXEL_STRUCT
			}
			free(imageDataMode[y]);
		}
		foutMode.close();
	};
#endif // NO_MODE

#ifndef NO_MEAN
	meanThread.join();
	std::thread meanWriteThread(meanWrite);
#endif // NO_MEAN
#ifndef NO_MEDIAN
	medianThread.join();
	std::thread medianWriteThread(medianWrite);
#endif // NO_MEDIAN
#ifndef NO_MODE
	modeThread.join();
	std::thread modeWriteThread(modeWrite);
#endif // NO_MODE

	for (int i = 0; i < num_images; i++) {
		for (int y = 0; y < height; y++) {
			free(images[i][y]);
		}
		free(images[i]);
	}
	free(images);

#ifndef NO_MEAN
	meanWriteThread.join();
	free(imageDataMean);
#endif // NO_MEAN
#ifndef NO_MEDIAN
	medianWriteThread.join();
	free(imageDataMedian);
#endif // NO_MEDIAN
#ifndef NO_MODE
	modeWriteThread.join();
	free(imageDataMode);
#endif // NO_MODE

#ifdef PRINT_PIXELS_PER_SECOND
#ifdef PRETTY_PRINT
	float divVal = (float)GetReadTime() / 1000.0f;
	std::cout << "Reading pixels at a rate of " << (float)GetPixels() / divVal << " pixels per second" << std::endl;
#else // PRETTY_PRINT
	std::cout << GetPixels() << " pixels read in " << GetReadTime() << " milliseconds" << std::endl;
#endif // PRETTY_PRINT
#endif // PRINT_PIXELS_PER_SECOND
#ifdef PRINT_NUM_MEANS
#ifdef PRETTY_PRINT
	divVal = (float)GetMeanTime() / 1000.0f;
	std::cout << "Calculating means at a rate of " << (float)GetMeans() / divVal << " means per second" << std::endl;
#else // PRETTY_PRINT
	std::cout << GetMeans() << " means calculated in " << GetMeanTime() << " milliseconds" << std::endl;
#endif // PRETTY_PRINT
#endif // PRINT_NUM_MEANS
#ifdef PRINT_NUM_MEDIANS
#ifdef PRETTY_PRINT
	divVal = (float)GetMedianTime() / 1000.0f;
	std::cout << "Calculating medians at a rate of " << (float)GetMedians() / divVal << " medians per second" << std::endl;
#else // PRETTY_PRINT
	std::cout << GetMedians() << " medians calculated in " << GetMedianTime() << " milliseconds" << std::endl;
#endif // PRETTY_PRINT
#endif // PRINT_NUM_MEDIANS
#ifdef PRINT_NUM_MODES
#ifdef PRETTY_PRINT
	divVal = (float)GetModeTime() / 1000.0f;
	std::cout << "Calculating modes at a rate of " << (float)GetModes() / divVal << " modes per second" << std::endl;
#else // PRETTY_PRINT
	std::cout << GetModes() << " modes calculated in " << GetModeTime() << " milliseconds" << std::endl;
#endif // PRETTY_PRINT
#endif // PRINT_NUM_MODES
}

// Video constructors work basically the same as the last ones, except we now have to init image to a null pointer
Video::Video(std::string newPath, std::string new_ffmpegPath, std::string fileName) {
	path = newPath;
	ffmpegPath = new_ffmpegPath;
#ifndef NO_MEAN
	std::string meanPath = path;
	meanPath.append("mean.bmp");
	meanOutput = meanPath;
#endif // NO_MEAN
#ifndef NO_MEDIAN
	std::string medianPath = path;
	medianPath.append("median.bmp");
	medianOutput = medianPath;
#endif // NO_MEDIAN
#ifndef NO_MODE
	std::string modePath = path;
	modePath.append("mode.bmp");
	modeOutput = modePath;
#endif // NO_MODE
	filename = fileName;
}

Video::Video(std::string newPath, std::string new_ffmpegPath, std::string fileName, std::string newMeanOutput, std::string newMedianOutput, std::string newModeOutput) {
	path = newPath;
	ffmpegPath = new_ffmpegPath;
#ifndef NO_MEAN
	meanOutput = newMeanOutput;
#endif // NO_MEAN
#ifndef NO_MEDIAN
	medianOutput = newMedianOutput;
#endif // NO_MEDIAN
#ifndef NO_MODE
	modeOutput = newModeOutput;
#endif // NO_MODE
	filename = fileName;
}

// Still deleting nothing as I have no pointer member variables
Video::~Video() {

}

// This actually processes the video
void Video::ActualClean(int& width, int& height) {
#ifndef NO_MEAN
	const char* mean = meanOutput.c_str();
#else // NO_MEAN
	const char* mean = " ";
#endif // NO_MEAN
#ifndef NO_MEDIAN
	const char* median = medianOutput.c_str();
#else // NO_MEDIAN
	const char* median = " ";
#endif // NO_MEDIAN
#ifndef NO_MODE
	const char* mode = modeOutput.c_str();
#else // NO_MODE
	const char* mode = " ";
#endif // NO_MODE

	avdevice_register_all();

	int ret;

	// Open input file context
	AVFormatContext * inctx = nullptr;
	ret = avformat_open_input(&inctx, (path + filename).c_str(), nullptr, nullptr);
	if (ret < 0) {
		// Fail to avforamt_open_input
		quick_exit(1);
	}

	// Retrive input stream information
	ret = avformat_find_stream_info(inctx, nullptr);
	if (ret < 0) {
		// Fail to avformat_find_stream_info
		quick_exit(1);
	}

	// Find primary video stream
	AVCodec* vcodec = nullptr;
	ret = av_find_best_stream(inctx, AVMEDIA_TYPE_VIDEO, -1, -1, (const AVCodec**) & vcodec, 0);
	if (ret < 0) {
		// Fail to av_find_best_stream
		quick_exit(1);
	}
	const int vstrm_idx = ret;
	AVStream* vstrm = inctx->streams[vstrm_idx];

	AVCodec* pCodec = (AVCodec*)avcodec_find_decoder(vstrm->codecpar->codec_id);
	AVCodecContext* context = avcodec_alloc_context3(pCodec);
	avcodec_parameters_to_context(context, vstrm->codecpar);
	context->pkt_timebase = vstrm->time_base;

	// Open video decoder context
	ret = avcodec_open2(context, vcodec, nullptr);
	if (ret < 0) {
		// Fail to avcodec_open2
		quick_exit(1);
	}

#ifdef OLD
	// Print input video stream informataion
	std::cout
		<< "infile: " << (path + filename).c_str() << "\n"
		<< "format: " << inctx->iformat->name << "\n"
		<< "vcodec: " << vcodec->name << "\n"
		<< "size:   " << vstrm->codecpar->width << 'x' << vstrm->codecpar->height << "\n"
		<< "fps:    " << av_q2d(context->framerate) << " [fps]\n"
		<< "length: " << av_rescale_q(vstrm->duration, vstrm->time_base, { 1,1000 }) / 1000. << " [sec]\n"
		<< "pixfmt: " << av_get_pix_fmt_name(context->pix_fmt) << "\n"
		<< "frame:  " << vstrm->nb_frames << "\n"
		<< std::flush;
#endif // OLD

	// Set width/height for return
	width = vstrm->codecpar->width;
	height = vstrm->codecpar->height;
	int frames = vstrm->nb_frames;

	// Allocate memory for pictures array
	COLORREF*** pictures = static_cast<COLORREF***>(calloc(vstrm->nb_frames, sizeof(COLORREF**)));

	// Init the last few ffmpeg objects needed
	AVFrame* frame = av_frame_alloc();
	bool eof = false;
	AVCodecParserContext* parser;
	AVPacket* pkt = av_packet_alloc();

	parser = av_parser_init(pCodec->id);
	if (!parser) {
		// Parser not found
		quick_exit(1);
	}

	// Allocate an AVFrame structure
	AVFrame* pFrameRGB = av_frame_alloc();

	int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width + 1, height + 1, 16);
	uint8_t* buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));

	// Decode video into pictures array
	do {
		if (av_read_frame(inctx, pkt) == AVERROR(EOF)) {
			eof = true;
		}
		if (!decode(context, frame, pkt, pictures, width, height, frames, buffer, pFrameRGB)) {
			break;
		}
	} while (!eof);

	// Free ffmpeg objects
	av_frame_free(&frame);
	avformat_close_input(&inctx);
	avcodec_free_context(&context);
	av_free(buffer);
	av_free(pFrameRGB);

	//Process pictures
	BitmapImage image(pictures, mean, median, mode, frames);

	image.Clean(width, height);

	// pictures gets freed in image.clean, so we don't have to free it here
}

#ifndef GET_PATH_FROM_FILESYSTEM
// Retrieves the name of the running executeble
std::string Video::GetEXEName() {
	// This is done differently on different systems
#if defined(PLATFORM_POSIX) || defined(__linux__)
	std::string sp;
	std::ifstream("/proc/self/comm") >> sp;
	return sp;

#elif defined(_WIN32) // defined(PLATFORM_POSIX) || defined(__linux__)
	char buf[MAX_PATH];
	GetModuleFileNameA(nullptr, buf, MAX_PATH);
	return buf;

#else // defined(_WIN32)
	static_assert(false, "unrecognized platform");
#endif // defined(PLATFORM_POSIX) || defined(__linux__)
}
#endif // GET_PATH_FROM_FILESYSTEM

// Just like with the other two, this just toggles on/off returns
void Video::Clean(int& width, int& height) {
	ActualClean(width, height);
}

void Video::Clean() {
	int width = -1, height = -1;
	ActualClean(width, height);
}

COLORREF getPixel(AVFrame* pFrame, short x, short y) {
	int off = ((y - 1) * pFrame->linesize[0]) + (3 * (x - 1));
	COLORREF px = RGB(pFrame->data[0][off], pFrame->data[0][off + 1], pFrame->data[0][off + 2]);

	return px;
}

bool decode(AVCodecContext* dec_ctx, AVFrame* frame, AVPacket* pkt, COLORREF*** pictures, int width, int height, int frames, uint8_t* buffer, AVFrame* pFrameRGB) {
	if (dec_ctx->frame_number >= frames) {
		return false;
	}

	int ret;

	ret = avcodec_send_packet(dec_ctx, pkt);
	if (ret < 0) {
		// Error sending a packet for decoding
		quick_exit(1);
	}

	while (ret >= 0) {
		ret = avcodec_receive_frame(dec_ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			return true;
		}
		else if (ret < 0) {
			// Error during decoding
			quick_exit(1);
		}

		// Convert image into an RGB image instead of whatever it decided to load
		SwsContext* scale_ctx;
		scale_ctx = sws_getContext(dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt, dec_ctx->width, dec_ctx->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
		av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, width, height, 1);
		sws_scale(scale_ctx, frame->data, frame->linesize, 0, dec_ctx->height, pFrameRGB->data, pFrameRGB->linesize);
		sws_freeContext(scale_ctx);

#ifdef OLD
		printf("saving frame %3d\n", dec_ctx->frame_number);
		fflush(stdout);
#endif // OLD

		// Save current frame into pictures using the same nested array method as earier
		// Funky +/- 1 as getPixel starts at 1,1, but the array has to start at 0,0
		COLORREF** currentData = static_cast<COLORREF**>(calloc(height, sizeof(COLORREF*)));
		for (int y = 1; y < height + 1; y++) {
			COLORREF* currentMiniData = static_cast<COLORREF*>(calloc(width, sizeof(COLORREF)));
			for (int x = 1; x < width + 1; x++) {
				COLORREF currentPixel = getPixel(pFrameRGB, x, y);
				currentMiniData[x - 1] = currentPixel;
			}
			// This is flipped as ffmpeg automatically loads the image upside-down
			currentData[height - y] = currentMiniData;
		}

		// Minus one here as dec_ctx->frame_number starts at 1, not 0
		pictures[dec_ctx->frame_number - 1] = currentData;
	}

	return true;
}
