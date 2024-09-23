#include "pch.h"
#include <Windows.h>
#include <winhttp.h>
#include <format>
#include <stdexcept>
#include <utility>
#include "hls.h"
#pragma comment(lib, "winhttp")

namespace {
	std::tuple<std::wstring, bool, std::wstring> ParseUrl(std::wstring_view url) {
		auto i = url.find(L'/');
		auto j = url.find(L'/', i + 2);
		if (i == std::wstring_view::npos || j == std::wstring_view::npos) {
			throw std::runtime_error("invalid URL");
		}
		auto serverName = std::wstring(url.begin() + i + 2, url.begin() + j);
		auto path = std::wstring(url.begin() + j, url.end());
		return { serverName, url[4] == L's', path };
	}

	template<typename T>
	void ThrowIfError(T t) {
		if (!t) {
			throw std::runtime_error("WinHttp error");
		}
	}
}

std::string hls::DownloadManifest(std::wstring_view url) {
	std::string rv;
	HINTERNET session = nullptr;
	HINTERNET connection = nullptr;
	HINTERNET request = nullptr;
	try {
		auto [serverName, isSecure, path] = ParseUrl(url);
		HINTERNET session = WinHttpOpen(L"Disney Exercise", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
		ThrowIfError(session);
		HINTERNET connection = WinHttpConnect(session, serverName.data(), isSecure ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT, 0);
		ThrowIfError(connection);
		HINTERNET request = WinHttpOpenRequest(connection, L"GET", path.data(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, isSecure ? WINHTTP_FLAG_SECURE : 0);
		ThrowIfError(request);
		ThrowIfError(WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0));
		ThrowIfError(WinHttpReceiveResponse(request, nullptr));
		rv.resize(9999, '\0');
		DWORD n;
		ThrowIfError(WinHttpReadData(request, &rv[0], static_cast<DWORD>(rv.size()), &n));
		rv.resize(n);
	} catch (std::exception const& ex) {
		DWORD error = GetLastError();
		std::string s = std::format("{}:  error code {}\n", ex.what(), error);
		OutputDebugStringA(s.c_str());
		rv.clear();
	}
	WinHttpCloseHandle(request);
	WinHttpCloseHandle(connection);
	WinHttpCloseHandle(session);
	return rv;
}
