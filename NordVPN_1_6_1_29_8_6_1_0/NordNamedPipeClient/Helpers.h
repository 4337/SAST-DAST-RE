#pragma once

#include <string>

using binary_string = std::basic_string<unsigned char>;

const std::string TP_SERVICE_SERVER = "\\\\.\\pipe\\nord-security\\v1\\ThreatProtectionService\\Server"; //change 2 \nord-security\v1\NordVpnApp\Server
const std::string NORD_VPN_SERVICE_SERVER = "\\\\.\\pipe\\nord-security\\v1\\NordVpnService\\Server";

struct Options {
	std::string dll;
};

Options parse(int argc, char** argv) noexcept(false);
