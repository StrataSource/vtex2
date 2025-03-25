
#include <cstdint>
#include <cstddef>
#include <climits>

#include "gtest/gtest.h"

#include "common/lwiconv.hpp"

using namespace lwiconv;

template<typename T>
static void fillPattern(T* buf, const T (&pattern)[MAX_CHANNELS], int w, int h, int channels) {
	for (int i = 0; i < w * h; ++i) {
		for (int j = 0; j < channels; ++j)
			buf[i * channels + j] = pattern[j];
	}
}

template<typename T>
struct Buffer {
	T* data;
	int w, h, c;
	Buffer(int w, int h, int channels) {
		data = (T*)malloc(w * h * channels * sizeof(T));
		this->w = w;
		this->h = h;
		this->c = channels;
	}

	~Buffer() {
		free(data);
	}

	void fill(const T(&pattern)[MAX_CHANNELS]) {
		fillPattern<T>((T*)data, pattern, w, h, c);
	}

	void check(const T(&pattern)[MAX_CHANNELS]) {
		for (int i = 0; i < w * h; ++i) {
			for (int j = 0; j < c; ++j) {
				if (data[i*c+j] != pattern[j])
					abort();
				ASSERT_EQ(data[i*c+j], pattern[j]);
			}
		}
	}
};

template<typename Tin, typename Tout>
void runTest(int width, int height, int inChannels, int outChannels, const Tin (&ipattern)[MAX_CHANNELS], const Tout (&opattern)[MAX_CHANNELS], int inBufChannels = -1, int inStride = -1, int outStride = -1,
	const PixelF& channelDefaults = {0,0,0,1})
{
	Buffer<Tin> b(width, height, inBufChannels < 0 ? inChannels : inBufChannels);
	b.fill(ipattern);

	Buffer<Tout> o(width, height, outChannels);

	convert_generic<Tin, Tout>(b.data, o.data, width, height, inChannels, outChannels, inStride, outStride, channelDefaults);

	o.check(opattern);
}

TEST(ImageTests, Basic8To16)
{
	runTest<uint8_t, uint16_t>(32, 32, 4, 4, {0xFF, 0, 0xFF, 0}, {0xFFFF, 0, 0xFFFF, 0});
	runTest<uint8_t, uint16_t>(32, 16, 4, 4, {0xFF, 0, 0xFF, 0}, {0xFFFF, 0, 0xFFFF, 0});
	runTest<uint8_t, uint16_t>(32, 128, 4, 4, {0xFF, 0, 0xFF, 0}, {0xFFFF, 0, 0xFFFF, 0});
	runTest<uint8_t, uint16_t>(32, 7, 4, 4, {0xFF, 0, 0xFF, 0}, {0xFFFF, 0, 0xFFFF, 0});
	runTest<uint8_t, uint16_t>(2, 7, 4, 4, {0, 0xFF, 0xFF, 0}, {0, 0xFFFF, 0xFFFF, 0});
	runTest<uint8_t, uint16_t>(2, 1, 4, 4, {0, 0, 0xFF, 0}, {0, 0, 0xFFFF, 0});
	runTest<uint8_t, uint16_t>(45, 177, 4, 4, {0xFF, 0, 0xFF, 0}, {0xFFFF, 0, 0xFFFF, 0});
	runTest<uint8_t, uint16_t>(1024, 999, 4, 4, {0xFF, 0, 0xFF, 0}, {0xFFFF, 0, 0xFFFF, 0});
}

TEST(ImageTests, DiffChannels8To16)
{
	runTest<uint8_t, uint16_t>(32,	32, 4, 4, 	{0xFF, 	0, 		0xFF, 0}, 	{0xFFFF, 	0, 		0xFFFF, 0});
	runTest<uint8_t, uint16_t>(32,	16,	4, 4, 	{0xFF, 	0, 		0xFF, 0}, 	{0xFFFF, 	0, 		0xFFFF, 0});
	runTest<uint8_t, uint16_t>(32,	128,4, 3, 	{0xFF, 	0, 		0xFF, 0}, 	{0xFFFF, 	0, 		0xFFFF, 0});
	runTest<uint8_t, uint16_t>(32,	7, 	4, 2, 	{0xFF, 	0, 		0xFF, 0}, 	{0xFFFF, 	0, 		0,		0});
	runTest<uint8_t, uint16_t>(2,	7, 	4, 3, 	{0, 	0xFF, 	0xFF, 0xFF},{0, 		0xFFFF, 0xFFFF, 0});
	runTest<uint8_t, uint16_t>(2, 	1, 	2, 4, 	{0, 	0, 		0xFF, 0}, 	{0, 		0, 		0,		0xFFFF},	4, sizeof(uint8_t) * 4, -1, {0, 0, 0, 1.f});
	runTest<uint8_t, uint16_t>(45, 177, 3, 4, 	{0xFF, 	0, 		0xFF, 0}, 	{0xFFFF,	0, 		0xFFFF, 0xFFFF},	4, sizeof(uint8_t) * 4, -1, {0, 0, 0, 1.f});
	runTest<uint8_t, uint16_t>(1024, 999, 2, 2, {0xFF, 	0, 		0xFF, 0}, 	{0xFFFF,	0, 		0,		0});
}

TEST(ImageTests, Basic8To32)
{
	runTest<uint8_t, float>(32, 32, 4, 4, {0xFF, 0, 0xFF, 0}, {1.0f, 0, 1.0f, 0});
	runTest<uint8_t, float>(32, 32, 4, 4, {0xFF, 0, 128, 0}, {1.0f, 0, 128.f / 255.f, 0});
}

TEST(ImageTests, Basic32To16)
{
	runTest<float, uint16_t>(32, 32, 4, 4, {1.0f, 0, 1.0f, 0}, {0xFFFF, 0, 0xFFFF, 0});
	runTest<float, uint16_t>(5, 32, 4, 4, {1.0f, 0, 1.0f, 0}, {0xFFFF, 0, 0xFFFF, 0});
	runTest<float, uint16_t>(55, 55, 4, 4, {1.0f, 0.5f, 1.0f, 0}, {0xFFFF, uint16_t(0.5f * 0xFFFF), 0xFFFF, 0});
}

TEST(ImageTests, Basic32To8)
{
	runTest<float, uint8_t>(32, 32, 4, 4, {1.0f, 0, 1.0f, 0}, {0xFF, 0, 0xFF, 0});
	runTest<float, uint8_t>(5, 32, 4, 4, {1.0f, 0, 1.0f, 0}, {0xFF, 0, 0xFF, 0});
	runTest<float, uint8_t>(55, 55, 4, 4, {1.0f, 0.5f, 1.0f, 0}, {0xFF, uint16_t(0.5f * 0xFF), 0xFF, 0});
}

TEST(ImageTests, DiffChannels32To8)
{
	runTest<float, uint8_t>(32,	32, 2, 4, 	{0, 	0, 		0,	0}, 	{0, 	0, 		0xFF,	0xFF}, 4, sizeof(uint8_t) * 4, -1, {0, 0, 1.f, 1.f});
	runTest<float, uint8_t>(32,	32, 3, 4, 	{0, 	0, 		0,	0}, 	{0, 	0, 		0,		0xFF}, 4, sizeof(uint8_t) * 4, -1, {0, 0, 1.f, 1.f});
	runTest<float, uint8_t>(3200,	3200, 1, 4, 	{0, 	0, 		0,	0}, 	{0, 	0xFF, 	0,		0xFF}, 4, sizeof(uint8_t) * 4, -1, {0, 1.f, 0, 1.f});
}

TEST(ImageTests, Basic8To8)
{
	runTest<uint8_t, uint8_t>(32, 32, 4, 4, {0xFF, 0, 0xFF, 0}, {0xFF, 0, 0xFF, 0});
	runTest<uint8_t, uint8_t>(32, 32, 4, 4, {128, 0, 0xFF, 99}, {128, 0, 0xFF, 99});
}

TEST(ImageTests, BasicSwizzle)
{
	uint8_t img[4] = {1,2,3,4};
	ASSERT_TRUE(swizzle<uint8_t>(img, 1, 1, 4, make_swizzle(3, 2, 1, 0)));
	ASSERT_EQ(img[0], 4);
	ASSERT_EQ(img[1], 3);
	ASSERT_EQ(img[2], 2);
	ASSERT_EQ(img[3], 1);
}
