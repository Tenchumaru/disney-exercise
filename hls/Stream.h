#pragma once

#include <string>
#include <utility>
#include <vector>

namespace hls {
	struct Stream {
		int Bandwidth;
		int AverageBandwidth;
		std::vector<std::string> Codecs;
		std::pair<int, int> Resolution;
		float FrameRate;
		std::string Audio;
		std::string Video;
		std::string ClosedCaptions;
	};
}
