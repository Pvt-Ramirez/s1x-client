#pragma once

namespace demonware
{

	class bdMarketing final : public service
	{
	public:
		bdMarketing();

	private:
		void unk3(service_server* server, uint8_t type, byte_buffer* buffer) const;
	};

}
