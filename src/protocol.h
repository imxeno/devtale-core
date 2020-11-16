#pragma once

#include "packet_handler.h"

namespace devtale
{
	class protocol
	{
	public:
		static protocol* get();
		void send(char* packet) const;
		void send(const std::string& packet) const;
		void receive(char* packet) const;
		void receive(const std::string& packet) const;
		void set_packet_handler(packet_handler* packet_handler);


	private:
		protocol();

		void setup();
		static void hooked_send();
		static void hooked_receive();
		static void __stdcall on_packet_send(char* packet);
		static void __stdcall on_packet_receive(char* packet);
		static std::vector<char> to_utf8(char* packet);
		DWORD ptr_ = 0, send_ = 0, receive_ = 0;
		packet_handler* handler_;
	};
}
