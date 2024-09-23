#pragma once

#include <string>
#include <string_view>

namespace hls {
	std::string DownloadManifest(std::wstring_view url);
}
