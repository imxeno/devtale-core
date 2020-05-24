#pragma once
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace devtale
{
	class packet_handler
	{
	public:
		explicit packet_handler(boost::beast::websocket::stream<boost::asio::ip::tcp::socket>& ws);
		void on_packet_send(std::string packet) const;
		void on_packet_receive(std::string packet) const;

	private:
		boost::beast::websocket::stream<boost::asio::ip::tcp::socket>& ws;
	};
}
