#pragma once

#include <boost/system/error_code.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio.hpp>
#include <boost/beast/websocket.hpp>
#include <nlohmann/json.hpp>
#include <mutex>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace asio = boost::asio;

using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;
using error_code = boost::system::error_code;

namespace devtale {

	typedef std::function<void(json req, json::value_type id)> message_handler;

	class /*pepega_*/websocket_handler {
	public:
		websocket_handler(asio::io_context&);
		void connect();
		void read();
		void write(std::string);
		void handle_error(error_code);
		void handle_message();
	private:
		std::string host_ = "127.0.0.1";
		std::string port_ = "17171";
		std::string ws_target_ = "/";

		std::mutex write_mtx_;
		tcp::resolver resolver_;
		websocket::stream<tcp::socket> ws_;
		asio::io_context::strand strand_;
		boost::beast::multi_buffer read_buffer_;

		void rpc_ping(json req, json::value_type id);
		void rpc_scan(json req, json::value_type id);
		void rpc_peek(json req, json::value_type id);
		void rpc_poke(json req, json::value_type id);
		void rpc_ntpvm(json req, json::value_type id);
		void rpc_read(json req, json::value_type id);
		void rpc_write(json req, json::value_type id);
		void rpc_disabled_method(json req, json::value_type id);
		void rpc_psend(json req, json::value_type id);
		void rpc_precv(json req, json::value_type id);

		std::map<std::string, message_handler> message_handler_map;

	};

}