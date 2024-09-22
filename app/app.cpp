#include <iostream>
#include "../hls/download_manifest.h"

int main() {
	auto result = download_manifest(L"https://lw.bamgrid.com/2.0/hls/vod/bam/ms02/hls/dplus/bao/master_unenc_hdr10_all.m3u8");
	std::cout << result << std::endl;
}
