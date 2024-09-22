#pragma once

#include <string>

enum class MediaType { Audio, Video, Subtitles, ClosedCaptions };

struct Media {
	MediaType Type;
	std::string Uri;
	std::string GroupId;
	std::string Language;
	std::string Name;
	bool Default;
	bool Autoselect;
	std::string InstreamId;
	std::string Channels;
};
