#include <stdexcept>
#include <string>
#include <utility>
#include "MasterPlaylist.h"
#include "Stream.h"
#include "Frame.h"

using namespace hls;

namespace {
	bool GetNextSegment(std::string_view manifest, std::string_view& segment) {
		// This pointer manipulation is okay since segment is guaranteed to be in manifest.
		size_t i = segment.data() + segment.size() - manifest.data();
		if (i < manifest.size() && manifest[i] == '#') {
			// Look for the next octothorpe.  This implementation assumes they do not appear in the data.
			auto j = manifest.find('#', i + 1);
			segment = std::string_view(manifest.begin() + i, j == std::string::npos ? manifest.end() : manifest.begin() + j);
			return true;
		}
		return false;
	}

	bool GetNextField(std::string_view segment, std::string_view& field) {
		// This pointer manipulation is okay since field is guaranteed to be in segment.
		size_t i = field.data() + field.size() - segment.data();

		// If this isn't the last field, look for the next one.
		if (segment[i] != '\n') {
			auto j = segment.find_first_of("\n,", i + 1);
			auto quote = segment.find('"', i + 1);
			while (quote < j) {
				// There is a comma in quoted text.  Keep looking until one is found outside of quoted text.
				quote = segment.find('"', quote + 1);
				j = segment.find_first_of("\n,", quote + 1);
				quote = segment.find('"', quote + 1);
			}
			field = std::string_view(segment.begin() + i + 1, j == std::string::npos ? segment.end() : segment.begin() + j);
			return true;
		}
		return false;
	}

	std::pair<std::string_view, std::string_view> SplitField(std::string_view field) {
		// Find the first equal sign separating the name from the value.
		auto i = field.find('=');
		if (i == std::string_view::npos) {
			throw new std::runtime_error("invalid field");
		}

		// Remove quotes from quoted values.
		auto value = field[i + 1] == '"' ?
			std::string_view(field.begin() + i + 2, field.end() - 1) :
			std::string_view(field.begin() + i + 1, field.end());
		return { std::string_view(field.begin(), field.begin() + i), value };
	}

	void ParseCodecs(std::string_view value, std::vector<std::string>& codecs) {
		// Split the value string on commas to extract the codecs.
		size_t i = 0;
		for (size_t j; j = value.find(',', i), j != std::string_view::npos; i = j + 1) {
			codecs.emplace_back(value.substr(i, j));
		}
		codecs.emplace_back(value.substr(i));
	}
}

void MasterPlaylist::ParseMedia(std::string_view segment) {
	// Configure field with "#EXT-X-MEDIA" to start field iteration.
	auto field = std::string_view(segment.begin(), segment.begin() + 12);
	Media media{};
	while (GetNextField(segment, field)) {
		auto [name, value] = SplitField(field);
		if (name == "TYPE") {
			media.Type = value == "AUDIO" ?
				MediaType::Audio : value == "VIDEO" ?
				MediaType::Video : value == "SUBTITLES" ?
				MediaType::Subtitles : MediaType::ClosedCaptions;
		} else if (name == "URI") {
			media.Uri = value;
		} else if (name == "GROUP-ID") {
			media.GroupId = value;
		} else if (name == "LANGUAGE") {
			media.Language = value;
		} else if (name == "NAME") {
			media.Name = value;
		} else if (name == "DEFAULT") {
			media.Default = value == "YES";
		} else if (name == "AUTOSELECT") {
			media.Autoselect = value == "YES";
		} else if (name == "INSTREAM-ID") {
			media.InstreamId = value;
		} else if (name == "CHANNELS") {
			media.Channels = value;
		}
	}
	medias.emplace_back(std::move(media));
}

void MasterPlaylist::ParseStream(std::string_view segment) {
	// Configure field with "#EXT-X-STREAM-INF" to start field iteration.
	auto field = std::string_view(segment.begin(), segment.begin() + 17);
	Stream stream{};
	while (GetNextField(segment, field)) {
		auto [name, value] = SplitField(field);
		if (name == "BANDWIDTH") {
			stream.Bandwidth = std::stoi(value.data());
		} else if (name == "AVERAGE-BANDWIDTH") {
			stream.AverageBandwidth = std::stoi(value.data());
		} else if (name == "CODECS") {
			ParseCodecs(value, stream.Codecs);
		} else if (name == "RESOLUTION") {
			// Resolutions are of the form "#x#".
			auto i = value.find('x');
			stream.Resolution = { std::stoi(value.data()), std::stoi(value.data() + i + 1) };
		} else if (name == "FRAME-RATE") {
			stream.FrameRate = std::stof(value.data());
		} else if (name == "AUDIO") {
			stream.Audio = value;
		} else if (name == "VIDEO") {
			stream.Video = value;
		} else if (name == "CLOSED-CAPTIONS") {
			stream.ClosedCaptions = value;
		}
	}
	streams.emplace_back(std::move(stream));
}

void MasterPlaylist::ParseFrame(std::string_view segment) {
	// Configure field with "#EXT-X-I-FRAME-STREAM-INF" to start field iteration.
	auto field = std::string_view(segment.begin(), segment.begin() + 25);
	Frame frame;
	while (GetNextField(segment, field)) {
		auto [name, value] = SplitField(field);
		if (name == "BANDWIDTH") {
			frame.Bandwidth = std::stoi(value.data());
		} else if (name == "AVERAGE-BANDWIDTH") {
			frame.AverageBandwidth = std::stoi(value.data());
		} else if (name == "CODECS") {
			ParseCodecs(value, frame.Codecs);
		} else if (name == "RESOLUTION") {
			// Resolutions are of the form "#x#".
			auto i = value.find('x');
			frame.Resolution = { std::stoi(value.data()), std::stoi(value.data() + i + 1) };
		} else if (name == "VIDEO") {
			frame.Video = value;
		} else if (name == "URI") {
			frame.Uri = value;
		}
	}
	frames.emplace_back(std::move(frame));
}

void MasterPlaylist::Parse(std::string_view manifest) {
	auto segment = std::string_view(manifest.begin(), manifest.begin());
	while (GetNextSegment(manifest, segment)) {
		if (segment.starts_with("#EXT-X-MEDIA:")) {
			ParseMedia(segment);
		} else if (segment.starts_with("#EXT-X-STREAM-INF:")) {
			ParseStream(segment);
		} else if (segment.starts_with("#EXT-X-I-FRAME-STREAM-INF:")) {
			ParseFrame(segment);
		}
	}
}
