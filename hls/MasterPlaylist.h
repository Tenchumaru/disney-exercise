#pragma once

#include <string_view>
#include <vector>
#include "Media.h"
#include "Stream.h"
#include "Frame.h"

namespace hls {
	class MasterPlaylist {
	public:
		void Parse(std::string_view manifest);
		__declspec(property(get = get_Streams)) std::vector<Stream>& Streams;
		std::vector<Stream>& get_Streams() { return streams; }

	private:
		std::vector<Media> medias;
		std::vector<Stream> streams;
		std::vector<Frame> frames;

		void ParseMedia(std::string_view segment);
		void ParseStream(std::string_view segment);
		void ParseFrame(std::string_view segment);
	};
}
