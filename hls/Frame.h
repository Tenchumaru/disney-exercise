#pragma once

#include <string>
#include <utility>
#include <vector>

namespace hls {
	struct Frame {
		int Bandwidth;
		int AverageBandwidth;
		std::vector<std::string> Codecs;
		std::pair<int, int> Resolution;
		std::string Video;
		std::string Uri;
	};
}
