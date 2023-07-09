
#include "pack.hpp"
#include "util.hpp"

using namespace pack;

template <int N>
void pack_channel(uint8_t* dstData, int destChannels, ChannelPack_t* channels, int w, int h) {

	ChannelPack_t *chan0, *chan1, *chan2, *chan3;
	float chan0const, chan1const, chan2const, chan3const;
	if constexpr (N > 0) {
		chan0 = &channels[0];
		chan0const = chan0->constant * 255.f;
	}
	if constexpr (N > 1) {
		chan1 = &channels[1];
		chan1const = chan1->constant * 255.f;
	}
	if constexpr (N > 2) {
		chan2 = &channels[2];
		chan2const = chan2->constant * 255.f;
	}
	if constexpr (N > 3) {
		chan3 = &channels[3];
		chan3const = chan3->constant * 256.f;
	}

	// This is pretty awful for performance. Not a lot of data locality and a lot of branching.
	// If this becomes a significant bottleneck, it should be reworked
	const int size = (w * h);
	for (int i = 0; i < size; ++i) {
		uint8_t* const pos = dstData + (i * destChannels);

		if constexpr (N > 0)
			pos[chan0->dstChan] = chan0->srcData ? chan0->srcData[(i * chan0->comps) + chan0->srcChan] : chan0const;
		if constexpr (N > 1)
			pos[chan1->dstChan] = chan1->srcData ? chan1->srcData[(i * chan1->comps) + chan1->srcChan] : chan1const;
		if constexpr (N > 2)
			pos[chan2->dstChan] = chan2->srcData ? chan2->srcData[(i * chan2->comps) + chan2->srcChan] : chan2const;
		if constexpr (N > 3)
			pos[chan3->dstChan] = chan3->srcData ? chan3->srcData[(i * chan3->comps) + chan3->srcChan] : chan3const;
	}
}

std::shared_ptr<imglib::Image>
pack::pack_image(int destChannels, ChannelPack_t* channels, int numChannels, int w, int h) {

	// Validate input data
	for (int i = 0; i < numChannels; ++i) {
		if (!channels[i].srcData)
			continue;
		if (channels[i].srcChan < channels[i].comps && channels[i].dstChan < destChannels)
			continue;
		return nullptr;
	}

	// Allocate image
	std::shared_ptr<imglib::Image> result = std::make_shared<imglib::Image>(imglib::UInt8, destChannels, w, h, false);

	if (numChannels == 1)
		pack_channel<1>(result->data<uint8_t>(), destChannels, channels, w, h);
	else if (numChannels == 2)
		pack_channel<2>(result->data<uint8_t>(), destChannels, channels, w, h);
	else if (numChannels == 3)
		pack_channel<3>(result->data<uint8_t>(), destChannels, channels, w, h);
	else if (numChannels == 4)
		pack_channel<4>(result->data<uint8_t>(), destChannels, channels, w, h);

	return result;
}
