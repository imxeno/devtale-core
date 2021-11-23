#pragma once
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>

#include "websocket_handler.h"

namespace devtale
{
	class packet_handler
	{
	public:
		explicit packet_handler(websocket_handler*);
		void on_packet_send(std::string packet) const;
		void on_packet_receive(std::string packet) const;

	private:
		websocket_handler* pepega_;
	};
}
