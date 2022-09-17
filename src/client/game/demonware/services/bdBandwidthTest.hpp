#pragma once

namespace demonware
{
	class bdBandwidthTest final : public service
	{
	public:
		bdBandwidthTest();

	private:
		void exec_task(service_server* server, const std::string& data) override;
	};
}
