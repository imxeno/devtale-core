#include "packet_handler.h"

#include "jsonrpc.hpp"

namespace devtale {

	packet_handler::packet_handler(boost::beast::websocket::stream<boost::asio::ip::tcp::socket>& ws) : ws(ws)
	{
	}

	void packet_handler::on_packet_send(std::string packet) const
	{
		try {
			if (ws.is_open()) {
				ws.write(boost::asio::buffer(jsonrpc::rpc_call("psend", packet)));
			}
		}
		catch (std::exception& error)
		{
			std::cout << error.what() << std::endl;
		}
	}

	void packet_handler::on_packet_receive(std::string packet) const
	{
		try {
			if (ws.is_open()) {
				ws.write(boost::asio::buffer(jsonrpc::rpc_call("precv", packet)));
			}
		} catch (std::exception& error)
		{
			std::cout << error.what() << std::endl;
		}
	}
}
