#pragma once

namespace toast
{
	// Thread-safe: schedules onto main thread automatically.
	void show(const std::string& title, const std::string& description, const std::string& icon = "blacktransparent");

	// Convenience wrappers with appropriate icons
	void success(const std::string& title, const std::string& description);
	void error(const std::string& title, const std::string& description);
	void info(const std::string& title, const std::string& description);
	void warn(const std::string& title, const std::string& description);
}
