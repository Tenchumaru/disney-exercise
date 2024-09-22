#include "pch.h"
#include <format>
#include "download_manifest.h"
#include <Windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp")

static std::tuple<std::wstring, bool, std::wstring> parse_url(std::wstring_view url) {
	auto i = url.find(L'/');
	auto j = url.find(L'/', i + 2);
	auto serverName = std::wstring(url.begin() + i + 2, url.begin() + j);
	auto path = std::wstring(url.begin() + j, url.end());
	return { serverName, url[4] == L's', path };
}

std::string download_manifest(std::wstring_view url) {
	HINTERNET session = WinHttpOpen(L"Disney Exercise", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (session) {
		auto [serverName, isSecure, path] = parse_url(url);
		HINTERNET connection = WinHttpConnect(session, serverName.data(), isSecure ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT, 0);
		if (connection) {
			HINTERNET request = WinHttpOpenRequest(connection, L"GET", path.data(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, isSecure ? WINHTTP_FLAG_SECURE : 0);
			if (request) {
				if (WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
					if (WinHttpReceiveResponse(request, nullptr)) {
						std::string rv(9999, '\0');
						DWORD n;
						if (WinHttpReadData(request, &rv[0], static_cast<DWORD>(rv.size()), &n)) {
							rv.resize(n);
							return rv;
						}
					}
				}
				WinHttpCloseHandle(request);
			}
			WinHttpCloseHandle(connection);
		}
		WinHttpCloseHandle(session);
	}
	DWORD error = GetLastError();
	std::string s = std::format("error {}\n", error);
	OutputDebugStringA(s.c_str());
	return {};
}
