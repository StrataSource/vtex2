/**
 * lwiconv: Lightweight Image Conversion library.
 * Not designed to be particularly fast, just simple.
 */
#pragma once

#include <cstdint>
#include <cstddef>

namespace lwiconv
{
constexpr int MAX_CHANNELS = 4;

/**
 * Just represents a pixel. Assumed to be normalized unsigned float format [0-1]
 */
struct PixelF {
	float d[MAX_CHANNELS];
};

namespace detail {

template <typename T>
inline float tofloat(const T& t);

template<> inline float tofloat<uint8_t>(const uint8_t& t) {
	return float(t) / float(UINT8_MAX);
}

template<> inline float tofloat<uint16_t>(const uint16_t& t) {
	return float(t) / float(UINT16_MAX);
}

template<> inline float tofloat<float>(const float& t) {
	return t;
}

template <typename T>
inline T fromfloat(float p);

template<> inline uint8_t fromfloat<uint8_t>(float p) {
	return uint8_t(p * UINT8_MAX);
}

template<> inline uint16_t fromfloat<uint16_t>(float p) {
	return uint16_t(p * UINT16_MAX);
}

template<> inline float fromfloat<float>(float p) {
	return p;
}

template <typename T, int COMPS>
inline PixelF pixel_from_data(const T* pin, const PixelF& defs) {
	if constexpr (COMPS == 1)
		return {{tofloat(*pin), defs.d[1], defs.d[2], defs.d[3]}};
	else if constexpr (COMPS == 2)
		return {{tofloat(*pin), tofloat(pin[1]), defs.d[2], defs.d[3]}};
	else if constexpr (COMPS == 3)
		return {{tofloat(*pin), tofloat(pin[1]), tofloat(pin[2]), defs.d[3]}};
	else if constexpr (COMPS == 4)
		return {{tofloat(*pin), tofloat(pin[1]), tofloat(pin[2]), tofloat(pin[3])}};
}

template <typename T, int COMPS>
inline void pixel_to_data(T* pout, const PixelF& p) {
	static_assert(COMPS <= MAX_CHANNELS && COMPS > 0);
	if constexpr (COMPS == 1) {
		pout[0] = fromfloat<T>(p.d[0]);
	}
	else if constexpr (COMPS == 2) {
		pout[0] = fromfloat<T>(p.d[0]);
		pout[1] = fromfloat<T>(p.d[1]);
	}
	else if constexpr (COMPS == 3) {
		pout[0] = fromfloat<T>(p.d[0]);
		pout[1] = fromfloat<T>(p.d[1]);
		pout[2] = fromfloat<T>(p.d[2]);
	}
	else if constexpr (COMPS == 4) {
		pout[0] = fromfloat<T>(p.d[0]);
		pout[1] = fromfloat<T>(p.d[1]);
		pout[2] = fromfloat<T>(p.d[2]);
		pout[3] = fromfloat<T>(p.d[3]);
	}
}

}

/**
 * \brief Convert buffer from one color format to another
 * The input and output buffers are assumed to be the same dimensions.
 * \param in Pointer to the input buffer
 * \param out Pointer to the output buffer
 * \param w Width of the image
 * \param h Height of the image
 * \param inC Number of input channels
 * \param outC Number of output channels
 * \param inStride Input stride, in bytes. If set <= 0, it will be computed for you based on inC 
 * \param outStride Output stride, in bytes. If set <= 0, it will be computed for you based on outC
 * \param channelDefaults If inC < outC, the missing channel data from each input pixel will be defaulted to this. For example, if you're converting from
 *  an RGB_888 -> RGBA_8888 image, supplying {0,0,0,1} here will default the resulting alpha channel to 255
 */
template <typename Tin, typename Tout>
static void convert_generic(const void* in, void* out, int w, int h, int inC, int outC, int inStride = -1, int outStride = -1, const PixelF& channelDefaults = {0,0,0,0}) {
	const Tin* pin = static_cast<const Tin*>(in);
	Tout* pout = static_cast<Tout*>(out);

	// Compute stride if not provided
	if (inStride <= 0)
		inStride = inC * sizeof(Tin);
	if (outStride <= 0)
		outStride = outC * sizeof(Tout);

	using fnInConv = PixelF (*)(const Tin*, const PixelF&);
	constexpr fnInConv inConvFuncs[MAX_CHANNELS] = {
		detail::pixel_from_data<Tin, 1>,
		detail::pixel_from_data<Tin, 2>,
		detail::pixel_from_data<Tin, 3>,
		detail::pixel_from_data<Tin, 4>,
	};
	const fnInConv inConv = inConvFuncs[inC-1];

	using fnOutConv = void (*)(Tout*, const PixelF&);
	constexpr fnOutConv outConvFuncs[MAX_CHANNELS] = {
		detail::pixel_to_data<Tout, 1>,
		detail::pixel_to_data<Tout, 2>,
		detail::pixel_to_data<Tout, 3>,
		detail::pixel_to_data<Tout, 4>,
	};
	const fnOutConv outConv = outConvFuncs[outC-1];

	const size_t target = w * h;
	const size_t isb = inStride / sizeof(Tin);
	const size_t osb = outStride / sizeof(Tout);

	for (size_t i = 0; i < target; ++i, pin += isb, pout += osb)
		outConv(pout, inConv(pin, channelDefaults));
}

} // lwiconv
