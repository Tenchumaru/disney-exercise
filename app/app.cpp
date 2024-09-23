#include <algorithm>
#include <iostream>
#include "../hls/hls.h"
#include "../hls/MasterPlaylist.h"
#include "../hls/Stream.h"

namespace {
	bool CompareStreams(hls::Stream& left, hls::Stream& right) {
		if (left.Resolution.second < right.Resolution.second) {
			return true;
		} else if (right.Resolution.second < left.Resolution.second) {
			return false;
		}
		return left.Bandwidth < right.Bandwidth;
	}
}

int main() {
	// Download the master playlist manifest.
	auto result = hls::DownloadManifest(L"https://lw.bamgrid.com/2.0/hls/vod/bam/ms02/hls/dplus/bao/master_unenc_hdr10_all.m3u8");
	if (!result.empty()) {
		// Parse it into an instance of the MasterPlaylist class.
		hls::MasterPlaylist masterPlaylist;
		masterPlaylist.Parse(result);

		// Sort the streams by resolution and bandwidth.
		std::sort(masterPlaylist.Streams.begin(), masterPlaylist.Streams.end(), CompareStreams);

		// Print the streams.
		for (auto const& stream : masterPlaylist.Streams) {
			std::cout << stream.Resolution.first << 'x' << stream.Resolution.second << ' ' << stream.Bandwidth << std::endl;
		}
	}
}
