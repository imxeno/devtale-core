#include "packet_handler.h"

#include "jsonrpc.hpp"

namespace devtale {

	packet_handler::packet_handler(websocket_handler* pepega) : pepega_(pepega)
	{
	}

	void packet_handler::on_packet_send(std::string packet) const
	{
		pepega_->write(jsonrpc::rpc_call("psend", packet));
	}

	void packet_handler::on_packet_receive(std::string packet) const
	{
		pepega_->write(jsonrpc::rpc_call("precv", packet));
	}
}
