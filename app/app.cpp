#include <iostream>
#include "../hls/hls.h"
#include "../hls/MasterPlaylist.h"

int main() {
	auto result = hls::DownloadManifest(L"https://lw.bamgrid.com/2.0/hls/vod/bam/ms02/hls/dplus/bao/master_unenc_hdr10_all.m3u8");
	if (!result.empty()) {
		std::cout << result << std::endl;
		hls::MasterPlaylist masterPlaylist;
		masterPlaylist.Parse(result);
	}
}
