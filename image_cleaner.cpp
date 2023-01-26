#include "image_cleaner.h"

void Image::Pixel::save_on_file(std::ofstream& fout) {
	fout.write((char*)&this->blue, sizeof(uint8_t));
	fout.write((char*)&this->green, sizeof(uint8_t));
	fout.write((char*)&this->red, sizeof(uint8_t));
}

Image::Image(std::string newPath) {
#ifdef PRINT_PIXELS_PER_SECOND
	m_pixels = 0;
	readTime = 0;
#endif
#ifdef PRINT_NUM_MEANS
	means = 0;
	meanTime = 0;
#endif
#ifdef PRINT_NUM_MEDIANS
	medians = 0;
	medianTime = 0;
#endif
#ifdef PRINT_NUM_MODES
	modes = 0;
	modeTime = 0;
#endif
	path = newPath;

#ifndef NO_MEAN
	std::string meanPath = path;
	meanPath.append("mean.bmp");
	meanOutput = meanPath;
#endif
#ifndef NO_MEDIAN
	std::string medianPath = path;
	medianPath.append("median.bmp");
	medianOutput = medianPath;
#endif
#ifndef NO_MODE
	std::string modePath = path;
	modePath.append("mode.bmp");
	modeOutput = modePath;
#endif
}

Image::Image(std::string newPath, const char* newMeanOutput, const char* newMedianOutput, const char* newModeOutput) {
#ifdef PRINT_PIXELS_PER_SECOND
	m_pixels = 0;
	readTime = 0;
#endif
#ifdef PRINT_NUM_MEANS
	means = 0;
	meanTime = 0;
#endif
#ifdef PRINT_NUM_MEDIANS
	medians = 0;
	medianTime = 0;
#endif
#ifdef PRINT_NUM_MODES
	modes = 0;
	modeTime = 0;
#endif
	path = newPath;

#ifndef NO_MEAN
	std::string meanPath = path;
	meanPath.append(newMeanOutput);
	meanOutput = meanPath;
#endif
#ifndef NO_MEDIAN
	std::string medianPath = path;
	medianPath.append(newMedianOutput);
	medianOutput = medianPath;
#endif
#ifndef NO_MODE
	std::string modePath = path;
	modePath.append(newModeOutput);
	modeOutput = modePath;
#endif
}

Image::~Image() {

}

COLORREF** Image::ReadImage(WCHAR*, int&, int&) {
	return nullptr;// Virtual, so don't care
}

#ifndef NO_MEAN
COLORREF Image::mean(COLORREF*** colour, int x, int y, int num) {
	float red = 0, green = 0, blue = 0;

	for (int i = 0; i < num; i++) {
		COLORREF** cur = colour[i];
		red += GetRValue(cur[y][x]);
		green += GetGValue(cur[y][x]);
		blue += GetBValue(cur[y][x]);
	}

#ifdef PRINT_NUM_MEANS
	means++;
#endif

	return RGB(red / (float)num, green / (float)num, blue / (float)num);
}
#endif

#ifndef NO_MEDIAN
COLORREF Image::median(COLORREF*** colour, int x, int y, const int num) {
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
#endif

	for (int i = 0; i < num; i++) {
		redList[i] = GetRValue(colour[i][y][x]);
		greenList[i] = GetGValue(colour[i][y][x]);
		blueList[i] = GetBValue(colour[i][y][x]);
	}

	std::sort(redList, redList + num, std::greater<int>());
	std::sort(greenList, greenList + num, std::greater<int>());
	std::sort(blueList, blueList + num, std::greater<int>());

	int redVal, greenVal, blueVal;
	if (num % 2 == 1) {
		redVal = redList[(num + 1) / 2];
		greenVal = greenList[(num + 1) / 2];
		blueVal = blueList[(num + 1) / 2];
	}
	else {
		redVal = (redList[num / 2] + redList[num / 2 + 1]) / 2;
		greenVal = (greenList[num / 2] + greenList[num / 2 + 1]) / 2;
		blueVal = (blueList[num / 2] + blueList[num / 2 + 1]) / 2;
	}

	free(redList);
	free(greenList);
	free(blueList);

#ifdef PRINT_NUM_MEDIANS
	medians++;
#endif

	return RGB(redVal, greenVal, blueVal);
}

COLORREF Image::fast_median(COLORREF*** colour, int x, int y, const int num) {
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
#endif

	for (int i = 0; i < num; i++) {
		redList[i] = GetRValue(colour[i][y][x]);
		greenList[i] = GetGValue(colour[i][y][x]);
		blueList[i] = GetBValue(colour[i][y][x]);
	}

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

	if (redMax <= redMin + FAST_MEDIAN_RANGE) {
		redVal = (redMin + redMax) / 2;
	}
	else {
		std::sort(redList, redList + num, std::greater<int>());

		if (num % 2 == 1) {
			redVal = redList[(num + 1) / 2];
		}
		else {
			redVal = (redList[num / 2] + redList[num / 2 + 1]) / 2;
		}
	}

	if (greenMax <= greenMin + FAST_MEDIAN_RANGE) {
		greenVal = (greenMin + greenMax) / 2;
	}
	else {
		std::sort(greenList, greenList + num, std::greater<int>());

		if (num % 2 == 1) {
			greenVal = greenList[(num + 1) / 2];
		}
		else {
			greenVal = (greenList[num / 2] + greenList[num / 2 + 1]) / 2;
		}
	}

	if (blueMax <= blueMin + FAST_MEDIAN_RANGE) {
		blueVal = (blueMin + blueMax) / 2;
	}
	else {
		std::sort(blueList, blueList + num, std::greater<int>());

		if (num % 2 == 1) {
			blueVal = blueList[(num + 1) / 2];
		}
		else {
			blueVal = (blueList[num / 2] + blueList[num / 2 + 1]) / 2;
		}
	}

	free(redList);
	free(greenList);
	free(blueList);

#ifdef PRINT_NUM_MEDIANS
	medians++;
#endif

	return RGB(redVal, greenVal, blueVal);
}
#endif

#ifndef NO_MODE
COLORREF Image::mode(COLORREF*** colour, int x, int y, const int num) {
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
#endif

	for (int i = 0; i < num; i++) {
		redList[i] = GetRValue(colour[i][y][x]);
		greenList[i] = GetGValue(colour[i][y][x]);
		blueList[i] = GetBValue(colour[i][y][x]);
	}

	std::sort(redList, redList + num, std::greater<int>());
	std::sort(greenList, greenList + num, std::greater<int>());
	std::sort(blueList, blueList + num, std::greater<int>());

	int redMode = *redList;
	int greenMode = *greenList;
	int blueMode = *blueList;

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

	COLORREF ret = RGB(redMode, greenMode, blueMode);

	free(redList);
	free(greenList);
	free(blueList);

#ifdef PRINT_NUM_MODES
	modes++;
#endif

	return ret;
}

COLORREF Image::fast_mode(COLORREF*** colour, int x, int y, const int num) {
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
#endif

	for (int i = 0; i < num; i++) {
		redList[i] = GetRValue(colour[i][y][x]);
		greenList[i] = GetGValue(colour[i][y][x]);
		blueList[i] = GetBValue(colour[i][y][x]);
	}

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

	if (redMax <= redMin + FAST_MODE_RANGE && greenMax <= greenMin + FAST_MODE_RANGE && blueMax <= blueMin + FAST_MODE_RANGE) {
		redMode = (redMin + redMax) / 2;
		greenMode = (greenMin + greenMax) / 2;
		blueMode = (blueMin + blueMax) / 2;
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
#endif

	return ret;
}
#endif

void Image::ActualClean(int&, int&) {
	//Virtual, so don't care
}

void Image::Clean(int& width, int& height) {
	ActualClean(width, height);
}

void Image::Clean() {
	int width = -1, height = -1;
	ActualClean(width, height);
}

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
#endif

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
#endif

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
#endif

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
#endif

std::string Image::GetPath() {
	return path;
}

#ifndef NO_MEAN
std::string Image::GetMeanOutput() {
	return meanOutput;
}
#endif

#ifndef NO_MEDIAN
std::string Image::GetMedianOutput() {
	return medianOutput;
}
#endif

#ifndef NO_MODE
std::string Image::GetModeOutput() {
	return modeOutput;
}
#endif

void BitmapImage::BmpHeader::save_on_file(std::ofstream& fout) {
	fout.write(this->bitmapSignatureBytes, 2);
	fout.write((char*)&this->sizeOfBitmapFile, sizeof(uint32_t));
	fout.write((char*)&this->reservedBytes, sizeof(uint32_t));
	fout.write((char*)&this->pixelDataOffset, sizeof(uint32_t));
}

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

COLORREF** BitmapImage::ReadImage(WCHAR* filename, int& width, int& height) {
	HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	HDC hdc = CreateCompatibleDC(NULL);
	(HBITMAP)SelectObject(hdc, hBitmap);

	BITMAP bmp;
	GetObject(hBitmap, sizeof(bmp), &bmp);

	BITMAPINFO info;
	info.bmiHeader.biSize = sizeof(info.bmiHeader);
	info.bmiHeader.biWidth = bmp.bmWidth;
	info.bmiHeader.biHeight = bmp.bmHeight;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 24;
	info.bmiHeader.biCompression = BI_RGB;

	COLORREF** data = static_cast<COLORREF**>(malloc(bmp.bmHeight * sizeof(COLORREF*)));

	size_t pixelSize = info.bmiHeader.biBitCount / 8;
	size_t scanlineSize = (pixelSize * info.bmiHeader.biWidth + 3) & ~3;
	size_t bitmapSize = bmp.bmHeight * scanlineSize;

	std::vector<BYTE> pixels(bitmapSize);
	GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, &pixels[0], &info, DIB_RGB_COLORS);

#ifdef PRINT_PIXELS_PER_SECOND
	SetReadTime(clock());
#endif

	for (LONG y = 0; y < bmp.bmHeight; y++) {
		COLORREF* miniData = static_cast<COLORREF*>(malloc(bmp.bmWidth * sizeof(COLORREF)));

		for (LONG x = 0; x < bmp.bmWidth; x++) {
			size_t pixelOffset = y * scanlineSize + x * pixelSize;
			COLORREF color = RGB(
				pixels[pixelOffset + 2],
				pixels[pixelOffset + 1],
				pixels[pixelOffset + 0]);
			miniData[x] = color;

#ifdef PRINT_PIXELS_PER_SECOND
			SetPixels(GetPixels() + 1);
#endif
		}
		data[y] = miniData;
	}

#ifdef PRINT_PIXELS_PER_SECOND
	SetReadTime(clock() - GetReadTime());
#endif

	if (width == -1 && height == -1) {
		width = bmp.bmWidth;
		height = bmp.bmHeight;
	}
	else {
		width = min(width, bmp.bmWidth);
		height = min(height, bmp.bmHeight);
	}

	return data;
}

void BitmapImage::ActualClean(int& width, int& height) {
	std::vector<std::wstring> validPaths;

	for (const auto& entry : std::filesystem::directory_iterator(GetPath())) {
		std::string substring = entry.path().string().substr(strlen(entry.path().string().c_str()) - 4), strlen(entry.path().string().c_str());
		if (substring == ".bmp") {
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
#else
					std::wstring temp(entry.path().string().size(), L' ');
					temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
					validPaths.push_back(temp);
#endif
				}
#else
#ifndef NO_MODE
				if (entry.path().string() != GetModeOutput()) {
					std::wstring temp(entry.path().string().size(), L' ');
					temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
					validPaths.push_back(temp);
				}
#else
				std::wstring temp(entry.path().string().size(), L' ');
				temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
				validPaths.push_back(temp);
#endif
#endif
			}
#else
#ifndef NO_MEDIAN
			if (entry.path().string() != GetMedianOutput()) {
#ifndef NO_MODE
				if (entry.path().string() != GetModeOutput()) {
					std::wstring temp(entry.path().string().size(), L' ');
					temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
					validPaths.push_back(temp);
				}
#else
				std::wstring temp(entry.path().string().size(), L' ');
				temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
				validPaths.push_back(temp);
#endif
			}
#else
#ifndef NO_MODE
			if (entry.path().string() != GetModeOutput()) {
				std::wstring temp(entry.path().string().size(), L' ');
				temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
				validPaths.push_back(temp);
			}
#else
			quick_exit(1);
#endif
#endif
#endif
		}
	}

	int num_images = (int)validPaths.size();

	COLORREF*** images = static_cast<COLORREF***>(calloc(num_images, sizeof(COLORREF**)));
#ifdef CHECK_BAD_ALLOC
	if (images == nullptr) {
		quick_exit(1);
	}
#endif

	for (int i = 0; i < num_images; i++) {
		images[i] = ReadImage((WCHAR*)validPaths[i].c_str(), width, height);
	}

#ifndef NO_MEAN
	COLORREF** imageDataMean = static_cast<COLORREF**>(malloc(height * sizeof(COLORREF*)));
#endif
#ifndef NO_MEDIAN
	COLORREF** imageDataMedian = static_cast<COLORREF**>(malloc(height * sizeof(COLORREF*)));
#endif
#ifndef NO_MODE
	COLORREF** imageDataMode = static_cast<COLORREF**>(malloc(height * sizeof(COLORREF*)));
#endif
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
#endif

#ifndef NO_MEAN
	auto doMean = [&]() {
#ifdef PRINT_NUM_MEANS
		SetMeanTime(clock());
#endif

		for (int y = 0; y < height; y++) {
			COLORREF* tempDataMean = static_cast<COLORREF*>(malloc(width * sizeof(COLORREF)));
#ifdef CHECK_BAD_ALLOC
			if (tempDataMean == nullptr) {
				quick_exit(1);
			}
#endif
			for (int x = 0; x < width; x++) {
				tempDataMean[x] = mean(images, x, y, num_images);
			}
			imageDataMean[y] = tempDataMean;
		}

#ifdef PRINT_NUM_MEANS
		SetMeanTime(clock() - GetMeanTime());
#endif
	};
#endif

#ifndef NO_MEDIAN
	auto doMedian = [&]() {
#ifdef PRINT_NUM_MEDIANS
		SetMedianTime(clock());
#endif

		for (int y = 0; y < height; y++) {
			COLORREF* tempDataMedian = static_cast<COLORREF*>(malloc(width * sizeof(COLORREF)));
#ifdef CHECK_BAD_ALLOC
			if (tempDataMedian == nullptr) {
				quick_exit(1);
			}
#endif
			for (int x = 0; x < width; x++) {
#ifdef FAST_MEDIAN_MODE
				tempDataMedian[x] = fast_median(images, x, y, num_images);
#else
				tempDataMedian[x] = median(images, x, y, num_images);
#endif
			}
			imageDataMedian[y] = tempDataMedian;
		}

#ifdef PRINT_NUM_MEDIANS
		SetMedianTime(clock() - GetMedianTime());
#endif
	};
#endif

#ifndef NO_MODE
	auto doMode = [&]() {
#ifdef PRINT_NUM_MODES
		SetModeTime(clock());
#endif

		for (int y = 0; y < height; y++) {
			COLORREF* tempDataMode = static_cast<COLORREF*>(malloc(width * sizeof(COLORREF)));
#ifdef CHECK_BAD_ALLOC
			if (tempDataMode == nullptr) {
				quick_exit(1);
			}
#endif
			for (int x = 0; x < width; x++) {
#ifdef FAST_MEDIAN_MODE
				tempDataMode[x] = fast_mode(images, x, y, num_images);
#else
				tempDataMode[x] = mode(images, x, y, num_images);
#endif
			}
			imageDataMode[y] = tempDataMode;
		}

#ifdef PRINT_NUM_MODES
		SetModeTime(clock() - GetModeTime());
#endif
	};
#endif

#ifndef NO_MEAN
	std::thread meanThread(doMean);
#endif
#ifndef NO_MEDIAN
	std::thread medianThread(doMedian);
#endif
#ifndef NO_MODE
	std::thread modeThread(doMode);
#endif

	bmpInfoHeader.width = width;
	bmpInfoHeader.height = height;

	bmpHeader.sizeOfBitmapFile = width * height * bmpInfoHeader.colorDepth;

#ifndef NO_MEAN
	auto meanWrite = [&]() {
		std::ofstream foutMean(GetMeanOutput(), std::ios::binary);

		bmpHeader.save_on_file(foutMean);
		bmpInfoHeader.save_on_file(foutMean);

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				Pixel pixel{ GetBValue(imageDataMean[y][x]), GetGValue(imageDataMean[y][x]), GetRValue(imageDataMean[y][x]) };
				pixel.save_on_file(foutMean);
			}
			free(imageDataMean[y]);
		}
		foutMean.close();
	};
#endif

#ifndef NO_MEDIAN
	auto medianWrite = [&]() {
		std::ofstream foutMedian(GetMedianOutput(), std::ios::binary);

		bmpHeader.save_on_file(foutMedian);
		bmpInfoHeader.save_on_file(foutMedian);

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				Pixel pixel2{ GetBValue(imageDataMedian[y][x]), GetGValue(imageDataMedian[y][x]), GetRValue(imageDataMedian[y][x]) };
				pixel2.save_on_file(foutMedian);
			}
			free(imageDataMedian[y]);
		}
		foutMedian.close();
	};
#endif

#ifndef NO_MODE
	auto modeWrite = [&]() {
		std::ofstream foutMode(GetModeOutput(), std::ios::binary);

		bmpHeader.save_on_file(foutMode);
		bmpInfoHeader.save_on_file(foutMode);

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				Pixel pixel3{ GetBValue(imageDataMode[y][x]), GetGValue(imageDataMode[y][x]), GetRValue(imageDataMode[y][x]) };
				pixel3.save_on_file(foutMode);
			}
			free(imageDataMode[y]);
		}
		foutMode.close();
	};
#endif

#ifndef NO_MEAN
	meanThread.join();
	std::thread meanWriteThread(meanWrite);
#endif
#ifndef NO_MEDIAN
	medianThread.join();
	std::thread medianWriteThread(medianWrite);
#endif
#ifndef NO_MODE
	modeThread.join();
	std::thread modeWriteThread(modeWrite);
#endif

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
#endif
#ifndef NO_MEDIAN
	medianWriteThread.join();
	free(imageDataMedian);
#endif
#ifndef NO_MODE
	modeWriteThread.join();
	free(imageDataMode);
#endif

#ifdef PRINT_PIXELS_PER_SECOND
#ifdef PRETTY_PRINT
	float divVal = (float)GetReadTime() / 1000.0f;
	std::cout << "Reading pixels at a rate of " << (float)GetPixels() / divVal << " pixels per second" << std::endl;
#else
	std::cout << m_pixels << " pixels read in " << readTime << " milliseconds" << std::endl;
#endif
#endif
#ifdef PRINT_NUM_MEANS
#ifdef PRETTY_PRINT
	divVal = (float)GetMeanTime() / 1000.0f;
	std::cout << "Calculating means at a rate of " << (float)GetMeans() / divVal << " means per second" << std::endl;
#else
	std::cout << means << " means calculated in " << meanTime << " milliseconds" << std::endl;
#endif
#endif
#ifdef PRINT_NUM_MEDIANS
#ifdef PRETTY_PRINT
	divVal = (float)GetMedianTime() / 1000.0f;
	std::cout << "Calculating medians at a rate of " << (float)GetMedians() / divVal << " medians per second" << std::endl;
#else
	std::cout << medians << " medians calculated in " << medianTime << " milliseconds" << std::endl;
#endif
#endif
#ifdef PRINT_NUM_MODES
#ifdef PRETTY_PRINT
	divVal = (float)GetModeTime() / 1000.0f;
	std::cout << "Calculating modes at a rate of " << (float)GetModes() / divVal << " modes per second" << std::endl;
#else
	std::cout << modes << " modes calculated in " << modeTime << " milliseconds" << std::endl;
#endif
#endif
}

COLORREF** JPEGImage::ReadImage(WCHAR* filename, int& width, int& height) {
#ifdef PRINT_PIXELS_PER_SECOND
	SetReadTime(clock());
#endif
	int Width, Height, channels;
	std::wstring name = std::wstring(filename);
	std::string filePath = std::string(name.begin(), name.end());
	unsigned char* test = stbi_load(filePath.c_str(), &Width, &Height, &channels, 3);

	COLORREF** data = static_cast<COLORREF**>(calloc(Width * Height, sizeof(COLORREF*)));
	for (int i = 0; i < Height; i++) {
		COLORREF* tempData = static_cast<COLORREF*>(calloc(Width, sizeof(COLORREF)));
		for (int j = 0; j < Width; j++) {
			uint8_t r = *(test + 3 * j + 3 * i * Width);
			uint8_t g = *(test + 3 * j + 3 * i * Width + 1);
			uint8_t b = *(test + 3 * j + 3 * i * Width + 2);

			COLORREF cur = RGB((int)r, (int)g, (int)b);
			tempData[j] = cur;

#ifdef PRINT_PIXELS_PER_SECOND
			SetPixels(GetPixels() + 1);
#endif
		}
		data[i] = tempData;
	}

#ifdef PRINT_PIXELS_PER_SECOND
	SetReadTime(clock() - GetReadTime());
#endif

	if (width == -1 && height == -1) {
		width = Width;
		height = Height;
	}
	else {
		width = min(width, Width);
		height = min(height, Height);
	}

	return data;
}

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
#else
					std::wstring temp(entry.path().string().size(), L' ');
					temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
					validPaths.push_back(temp);
#endif
				}
#else
#ifndef NO_MODE
				if (entry.path().string() != GetModeOutput()) {
					std::wstring temp(entry.path().string().size(), L' ');
					temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
					validPaths.push_back(temp);
				}
#else
				std::wstring temp(entry.path().string().size(), L' ');
				temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
				validPaths.push_back(temp);
#endif
#endif
			}
#else
#ifndef NO_MEDIAN
			if (entry.path().string() != GetMedianOutput()) {
#ifndef NO_MODE
				if (entry.path().string() != GetModeOutput()) {
					std::wstring temp(entry.path().string().size(), L' ');
					temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
					validPaths.push_back(temp);
				}
#else
				std::wstring temp(entry.path().string().size(), L' ');
				temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
				validPaths.push_back(temp);
#endif
			}
#else
#ifndef NO_MODE
			if (entry.path().string() != GetModeOutput()) {
				std::wstring temp(entry.path().string().size(), L' ');
				temp.resize(mbstowcs(&temp[0], entry.path().string().c_str(), entry.path().string().size()));
				validPaths.push_back(temp);
			}
#else
			quick_exit(1);
#endif
#endif
#endif
		}
	}

	int num_images = (int)validPaths.size();

	COLORREF*** images = static_cast<COLORREF***>(calloc(num_images, sizeof(COLORREF**)));
#ifdef CHECK_BAD_ALLOC
	if (images == nullptr) {
		quick_exit(1);
	}
#endif

	for (int i = 0; i < num_images; i++) {
		images[i] = ReadImage((WCHAR*)validPaths[i].c_str(), width, height);
	}

#ifndef NO_MEAN
	COLORREF** imageDataMean = static_cast<COLORREF**>(malloc(height * sizeof(COLORREF*)));
#endif
#ifndef NO_MEDIAN
	COLORREF** imageDataMedian = static_cast<COLORREF**>(malloc(height * sizeof(COLORREF*)));
#endif
#ifndef NO_MODE
	COLORREF** imageDataMode = static_cast<COLORREF**>(malloc(height * sizeof(COLORREF*)));
#endif
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
#endif

#ifndef NO_MEAN
	auto doMean = [&]() {
#ifdef PRINT_NUM_MEANS
		SetMeanTime(clock());
#endif

		for (int y = 0; y < height; y++) {
			COLORREF* tempDataMean = static_cast<COLORREF*>(malloc(width * sizeof(COLORREF)));
#ifdef CHECK_BAD_ALLOC
			if (tempDataMean == nullptr) {
				quick_exit(1);
			}
#endif
			for (int x = 0; x < width; x++) {
				tempDataMean[x] = mean(images, x, y, num_images);
			}
			imageDataMean[y] = tempDataMean;
		}

#ifdef PRINT_NUM_MEANS
		SetMeanTime(clock() - GetMeanTime());
#endif
	};
#endif

#ifndef NO_MEDIAN
	auto doMedian = [&]() {
#ifdef PRINT_NUM_MEDIANS
		SetMedianTime(clock());
#endif

		for (int y = 0; y < height; y++) {
			COLORREF* tempDataMedian = static_cast<COLORREF*>(malloc(width * sizeof(COLORREF)));
#ifdef CHECK_BAD_ALLOC
			if (tempDataMedian == nullptr) {
				quick_exit(1);
			}
#endif
			for (int x = 0; x < width; x++) {
#ifdef FAST_MEDIAN_MODE
				tempDataMedian[x] = fast_median(images, x, y, num_images);
#else
				tempDataMedian[x] = median(images, x, y, num_images);
#endif
			}
			imageDataMedian[y] = tempDataMedian;
		}

#ifdef PRINT_NUM_MEDIANS
		SetMedianTime(clock() - GetMedianTime());
#endif
	};
#endif

#ifndef NO_MODE
	auto doMode = [&]() {
#ifdef PRINT_NUM_MODES
		SetModeTime(clock());
#endif

		for (int y = 0; y < height; y++) {
			COLORREF* tempDataMode = static_cast<COLORREF*>(malloc(width * sizeof(COLORREF)));
#ifdef CHECK_BAD_ALLOC
			if (tempDataMode == nullptr) {
				quick_exit(1);
			}
#endif
			for (int x = 0; x < width; x++) {
#ifdef FAST_MEDIAN_MODE
				tempDataMode[x] = fast_mode(images, x, y, num_images);
#else
				tempDataMode[x] = mode(images, x, y, num_images);
#endif
			}
			imageDataMode[y] = tempDataMode;
		}

#ifdef PRINT_NUM_MODES
		SetModeTime(clock() - GetModeTime());
#endif
	};
#endif

#ifndef NO_MEAN
	std::thread meanThread(doMean);
#endif
#ifndef NO_MEDIAN
	std::thread medianThread(doMedian);
#endif
#ifndef NO_MODE
	std::thread modeThread(doMode);
#endif

	bmpInfoHeader.width = width;
	bmpInfoHeader.height = height;

	bmpHeader.sizeOfBitmapFile = width * height * bmpInfoHeader.colorDepth;

#ifndef NO_MEAN
	auto meanWrite = [&]() {
		std::ofstream foutMean(GetMeanOutput(), std::ios::binary);

		bmpHeader.save_on_file(foutMean);
		bmpInfoHeader.save_on_file(foutMean);

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				Pixel pixel{ GetBValue(imageDataMean[y][x]), GetGValue(imageDataMean[y][x]), GetRValue(imageDataMean[y][x]) };
				pixel.save_on_file(foutMean);
			}
			free(imageDataMean[y]);
		}
		foutMean.close();
	};
#endif

#ifndef NO_MEDIAN
	auto medianWrite = [&]() {
		std::ofstream foutMedian(GetMedianOutput(), std::ios::binary);

		bmpHeader.save_on_file(foutMedian);
		bmpInfoHeader.save_on_file(foutMedian);

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				Pixel pixel2{ GetBValue(imageDataMedian[y][x]), GetGValue(imageDataMedian[y][x]), GetRValue(imageDataMedian[y][x]) };
				pixel2.save_on_file(foutMedian);
			}
			free(imageDataMedian[y]);
		}
		foutMedian.close();
	};
#endif

#ifndef NO_MODE
	auto modeWrite = [&]() {
		std::ofstream foutMode(GetModeOutput(), std::ios::binary);

		bmpHeader.save_on_file(foutMode);
		bmpInfoHeader.save_on_file(foutMode);

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				Pixel pixel3{ GetBValue(imageDataMode[y][x]), GetGValue(imageDataMode[y][x]), GetRValue(imageDataMode[y][x]) };
				pixel3.save_on_file(foutMode);
			}
			free(imageDataMode[y]);
		}
		foutMode.close();
	};
#endif

#ifndef NO_MEAN
	meanThread.join();
	std::thread meanWriteThread(meanWrite);
#endif
#ifndef NO_MEDIAN
	medianThread.join();
	std::thread medianWriteThread(medianWrite);
#endif
#ifndef NO_MODE
	modeThread.join();
	std::thread modeWriteThread(modeWrite);
#endif

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
#endif
#ifndef NO_MEDIAN
	medianWriteThread.join();
	free(imageDataMedian);
#endif
#ifndef NO_MODE
	modeWriteThread.join();
	free(imageDataMode);
#endif

#ifdef PRINT_PIXELS_PER_SECOND
#ifdef PRETTY_PRINT
	float divVal = (float)GetReadTime() / 1000.0f;
	std::cout << "Reading pixels at a rate of " << (float)GetPixels() / divVal << " pixels per second" << std::endl;
#else
	std::cout << m_pixels << " pixels read in " << readTime << " milliseconds" << std::endl;
#endif
#endif
#ifdef PRINT_NUM_MEANS
#ifdef PRETTY_PRINT
	divVal = (float)GetMeanTime() / 1000.0f;
	std::cout << "Calculating means at a rate of " << (float)GetMeans() / divVal << " means per second" << std::endl;
#else
	std::cout << means << " means calculated in " << meanTime << " milliseconds" << std::endl;
#endif
#endif
#ifdef PRINT_NUM_MEDIANS
#ifdef PRETTY_PRINT
	divVal = (float)GetMedianTime() / 1000.0f;
	std::cout << "Calculating medians at a rate of " << (float)GetMedians() / divVal << " medians per second" << std::endl;
#else
	std::cout << medians << " medians calculated in " << medianTime << " milliseconds" << std::endl;
#endif
#endif
#ifdef PRINT_NUM_MODES
#ifdef PRETTY_PRINT
	divVal = (float)GetModeTime() / 1000.0f;
	std::cout << "Calculating modes at a rate of " << (float)GetModes() / divVal << " modes per second" << std::endl;
#else
	std::cout << modes << " modes calculated in " << modeTime << " milliseconds" << std::endl;
#endif
#endif
}