#include "protocol.h"
#include "memory.h"
#include <iostream>
#include "ansistring.h"

namespace devtale
{
	protocol::protocol()
	{
		setup();
	}

	void protocol::set_packet_handler(packet_handler *handler)
	{
		this->handler_ = handler;
	}

	protocol* protocol::get()
	{
		static protocol* p;
		if (p == nullptr) p = new protocol();
		return p;
	}

#pragma optimize("", off)
	
	void protocol::setup()
	{
		// ReSharper disable StringLiteralTypo
		const BYTE send_pattern_byte[] = {0x53, 0x56, 0x8B, 0xF2, 0x8B, 0xD8, 0xEB, 0x04};
		const auto send_pattern_mask = "xxxxxxxx";

		const BYTE receive_pattern_byte[] = {
			0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xFF, 0x53, 0x56, 0x57, 0x33, 0xC9, 0x89, 0x4D, 0xF4, 0x89, 0x4D, 0xF0,
			0x89, 0x55, 0xFC, 0x8B, 0xD8, 0x8B, 0x45, 0xFC
		};
		const auto receive_pattern_mask = "xxxxx?xxxxxxxxxxxxxxxxxxxxxx";

		const BYTE pre_20220412_receive_pattern_byte[] = {
			0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xFF, 0x53, 0x56, 0x57, 0x33, 0xC9, 0x89, 0x4D, 0xF4, 0x89, 0x55, 0xFC, 0x8B,
			0xD8, 0x8B, 0x45, 0xFC
		};
		const auto pre_20220412_receive_pattern_mask = "xxxxx?xxxxxxxxxxxxxxxx";

		const BYTE ptr_pattern_byte[] = {
			0xA1, 0xFF, 0xFF, 0xFF, 0xFF, 0x8B, 0x00, 0xBA, 0xFF, 0xFF, 0xFF, 0xFF, 0xE8, 0xFF, 0xFF, 0xFF, 0xFF, 0xE9,
			0xFF, 0xFF, 0xFF, 0xFF, 0xA1, 0xFF, 0xFF, 0xFF, 0xFF, 0x8B, 0x00, 0x8B, 0x40, 0x40
		};
		const auto ptr_pattern_mask = "x????xxx????x????x????x????xxxxx";
		// ReSharper restore StringLiteralTypo

		send_ = memory::find_pattern(send_pattern_byte, send_pattern_mask);
		std::cout << std::hex << "protocol> send at 0x" << send_ << std::dec << std::endl;

		receive_ = memory::find_pattern(receive_pattern_byte, receive_pattern_mask);
		if (receive_ == 0) receive_ = memory::find_pattern(pre_20220412_receive_pattern_byte, pre_20220412_receive_pattern_mask);
		std::cout << std::hex << "protocol> receive at 0x" << receive_ << std::dec << std::endl;

		ptr_ = *reinterpret_cast<DWORD*>(memory::find_pattern(ptr_pattern_byte, ptr_pattern_mask) + 0x01);
		std::cout << std::hex << "protocol> ptr at 0x" << ptr_ << std::dec << std::endl;
		memory::detour(reinterpret_cast<BYTE*>(send_), reinterpret_cast<BYTE*>(hooked_send), 16);
		memory::detour(reinterpret_cast<BYTE*>(receive_), reinterpret_cast<BYTE*>(hooked_receive), 14);
	}

	void __stdcall protocol::on_packet_send(char* packet)
	{
		const std::string ps(&to_utf8(packet)[0]);
		get()->handler_->on_packet_send(ps);
	}

	void __stdcall protocol::on_packet_receive(char* packet)
	{
		const std::string ps(&to_utf8(packet)[0]);
		get()->handler_->on_packet_receive(ps);
	}

	std::vector<char> protocol::to_utf8(char* packet)
	{
		const auto wbuffer_size = MultiByteToWideChar(1250, 0, packet, -1, nullptr, 0);
		std::vector<wchar_t> wbuffer(wbuffer_size);
		MultiByteToWideChar(1250, 0, packet, -1, &wbuffer[0], wbuffer_size);
		const auto buffer_size = WideCharToMultiByte(CP_UTF8, 0, &wbuffer[0], wbuffer_size, nullptr, 0, 0, nullptr);
		std::vector<char> buffer(buffer_size);
		WideCharToMultiByte(CP_UTF8, 0, &wbuffer[0], wbuffer_size, &buffer[0], buffer_size, 0, nullptr);
		return std::move(buffer);
	}

	void protocol::send(char* packet) const
	{
		std::cout << "protocol> sending packet '" << packet << "'" << std::endl;
		// ReSharper disable CppDeclaratorNeverUsed
		// ReSharper disable CppUseAuto
		char* nt_packet = ansistring(packet).get();

		DWORD network_ptr = ptr_;
		DWORD network_send = send_;
		// ReSharper restore CppUseAuto
		// ReSharper restore CppDeclaratorNeverUsed

		__asm
		{
			mov edx, nt_packet
			mov eax, dword ptr ds : [network_ptr]
			mov eax, dword ptr ds : [eax]
			mov eax, dword ptr ds : [eax]
			call network_send
		}
	}

	void protocol::receive(char* packet) const
	{
		std::cout << "protocol> injecting received packet '" << packet << "'" << std::endl;
		// ReSharper disable CppDeclaratorNeverUsed
		// ReSharper disable CppUseAuto
		char* nt_packet = ansistring(packet).get();

		DWORD network_ptr = ptr_;
		DWORD network_receive = receive_;
		// ReSharper restore CppUseAuto
		// ReSharper restore CppDeclaratorNeverUsed

		__asm
		{
			mov eax, dword ptr ss : [network_ptr]
			mov eax, dword ptr ss : [eax]
			mov eax, dword ptr ss : [eax]
			mov eax, dword ptr ds : [eax + 0x34]
			mov edx, nt_packet
			call network_receive
		}
	}

	void protocol::send(const std::string& packet) const
	{
		const auto size = packet.length() + 1;
		const auto str = new char[size];
		strcpy_s(str, size, packet.c_str());
		send(str);
		delete[] str;
	}

	void protocol::receive(const std::string& packet) const
	{
		const auto size = packet.length() + 1;
		const auto str = new char[size];
		strcpy_s(str, size, packet.c_str());
		receive(str);
		delete[] str;
	}

	void protocol::hooked_send()
	{
		char* packet;
		_asm
		{
			pushad
			pushfd
			mov packet, edx
		}

		get()->on_packet_send(packet);

		_asm
		{
			popfd
			popad
		}
	}

	void protocol::hooked_receive()
	{
		char* packet;

		_asm
		{
			pushad
			pushfd
			mov packet, edx
		}

		get()->on_packet_receive(packet);

		_asm
		{
			popfd
			popad
		}
	}
	
#pragma optimize("", on)
	
}