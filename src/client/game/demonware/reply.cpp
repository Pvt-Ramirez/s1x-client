#include <std_include.hpp>
#include "demonware.hpp"
#include <utils/cryptography.hpp>

namespace demonware
{

	std::string unencrypted_reply::data()
	{
		byte_buffer result;
		result.set_use_data_types(false);

		result.write_int32(static_cast<int>(this->buffer_.size()) + 2);
		result.write_bool(false);
		result.write_byte(this->type());
		result.write(this->buffer_);

		return result.get_buffer();
	}

	std::string encrypted_reply::data()
	{
		byte_buffer result;
		result.set_use_data_types(false);

		byte_buffer enc_buffer;
		enc_buffer.set_use_data_types(false);

		enc_buffer.write_uint32(static_cast<unsigned int>(this->buffer_.size()));	// service data size CHECKTHIS!!
		enc_buffer.write_byte(this->type());										// TASK_REPLY type
		enc_buffer.write(this->buffer_);											// service data

		auto aligned_data = enc_buffer.get_buffer();
		auto size = aligned_data.size();
		size = ~15 & (size + 15); // 16 byte align
		aligned_data.resize(size);

		// seed
		std::string seed("\x5E\xED\x5E\xED\x5E\xED\x5E\xED\x5E\xED\x5E\xED\x5E\xED\x5E\xED", 16);

		// encrypt
		auto enc_data = utils::cryptography::aes::encrypt(aligned_data, seed, demonware::get_encrypt_key());

		// header : encrypted service data : hash
		static std::int32_t msg_count = 0;
		msg_count++;

		byte_buffer response;
		response.set_use_data_types(false);

		response.write_int32(30 + static_cast<int>(enc_data.size()));
		response.write_byte(static_cast<char>(0xAB));
		response.write_byte(static_cast<char>(0x85));
		response.write_int32(msg_count);
		response.write(16, seed.data());
		response.write(enc_data);

		// hash entire packet and append end
		unsigned int outlen = 20;
		auto hash_data = utils::cryptography::hmac_sha1::process(response.get_buffer(), demonware::get_hmac_key(), &outlen);
		hash_data.resize(8);
		response.write(8, hash_data.data());

		return response.get_buffer();
	}

	void remote_reply::send(bit_buffer* buffer, const bool encrypted)
	{
		std::unique_ptr<typed_reply> reply;

		if (encrypted) reply = std::make_unique<encrypted_reply>(this->type_, buffer);
		else reply = std::make_unique<unencrypted_reply>(this->type_, buffer);

		this->server_->send_reply(reply.get());
	}

	void remote_reply::send(byte_buffer* buffer, const bool encrypted)
	{
		std::unique_ptr<typed_reply> reply;

		if (encrypted) reply = std::make_unique<encrypted_reply>(this->type_, buffer);
		else reply = std::make_unique<unencrypted_reply>(this->type_, buffer);

		this->server_->send_reply(reply.get());
	}

}
