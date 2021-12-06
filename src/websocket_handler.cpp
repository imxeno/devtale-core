#include "websocket_handler.h"

#include "jsonrpc.hpp"
#include "utils.h"
#include "protocol.h"

namespace devtale {

	websocket_handler::websocket_handler(asio::io_context& ioc) : strand_(ioc), resolver_(ioc), ws_(ioc)
	{
		message_handler_map = std::map<std::string, message_handler> {
			std::make_pair("ping", [&](json req, json::value_type id) { rpc_ping(req, id); }),
#if DEVTALE_INSECURE
			std::make_pair("scan", [&](json req, json::value_type id) { rpc_scan(req, id); }),
			std::make_pair("peek", [&](json req, json::value_type id) { rpc_peek<BYTE>(req, id); }),
			std::make_pair("peekw", [&](json req, json::value_type id) { rpc_peek<WORD>(req, id); }),
			std::make_pair("peekd", [&](json req, json::value_type id) { rpc_peek<DWORD>(req, id); }),
			std::make_pair("poke", [&](json req, json::value_type id) { rpc_poke<BYTE>(req, id); }),
			std::make_pair("pokew", [&](json req, json::value_type id) { rpc_poke<WORD>(req, id); }),
			std::make_pair("poked", [&](json req, json::value_type id) { rpc_poke<DWORD>(req, id); }),
			std::make_pair("ntpvm", [&](json req, json::value_type id) { rpc_ntpvm(req, id); }),
			std::make_pair("read", [&](json req, json::value_type id) { rpc_read(req, id); }),
			std::make_pair("write", [&](json req, json::value_type id) { rpc_write(req, id); }),
#else
			std::make_pair("scan", [&](json req, json::value_type id) { rpc_disabled_method(req, id); }),
			std::make_pair("peek", [&](json req, json::value_type id) { rpc_disabled_method(req, id); }),
			std::make_pair("peekw", [&](json req, json::value_type id) { rpc_disabled_method(req, id); }),
			std::make_pair("peekd", [&](json req, json::value_type id) { rpc_disabled_method(req, id); }),
			std::make_pair("poke", [&](json req, json::value_type id) { rpc_disabled_method(req, id); }),
			std::make_pair("pokew", [&](json req, json::value_type id) { rpc_disabled_method(req, id); }),
			std::make_pair("poked", [&](json req, json::value_type id) { rpc_disabled_method(req, id); }),
			std::make_pair("ntpvm", [&](json req, json::value_type id) { rpc_disabled_method(req, id); }),
			std::make_pair("read", [&](json req, json::value_type id) { rpc_disabled_method(req, id); }),
			std::make_pair("write", [&](json req, json::value_type id) { rpc_disabled_method(req, id); }),
#endif
			std::make_pair("psend", [&](json req, json::value_type id) { rpc_psend(req, id); }),
			std::make_pair("precv", [&](json req, json::value_type id) { rpc_precv(req, id); })
		};


		// Port override
		{
			std::vector<char> temp_port(6);
			int r = GetEnvironmentVariable("devtale_port", &temp_port[0], 6);
			if (r != 0)
			{
				port_ = std::string(&temp_port[0]);
			}
		}

	}

	void websocket_handler::connect()
	{
		auto const resolved = resolver_.resolve(host_, port_);
		std::cout << "Pre async connect" << std::endl;
		asio::async_connect(ws_.next_layer(), resolved, [&](const error_code& error, const tcp::endpoint& ep) {
			std::cout << "async connect" << std::endl;
			if (error) {
				handle_error(error);
				return;
			}
			const auto ws_endpoint = host_ + ':' + std::to_string(ep.port());
			std::cout << "Pre async handshake" << std::endl;
			ws_.async_handshake(ws_endpoint, ws_target_, asio::bind_executor(strand_, [&](error_code error)
			{
					std::cout << "async handshake" << std::endl;
				if (error) {
					handle_error(error);
					return;
				}
				read();
			}));
		});

	}

	void websocket_handler::read() {
		read_buffer_ = boost::beast::multi_buffer();
		ws_.async_read(read_buffer_, asio::bind_executor(strand_, [&](error_code error, std::size_t bytes_transferred)
		{
				std::cout << bytes_transferred << std::endl;
			if (error) {
				handle_error(error);
				return;
			}
			handle_message();
			read();
		}));
	}

	void websocket_handler::write(std::string s) {
		if (ws_.is_open()) {
			write_mtx_.lock();
			try {
				ws_.write(boost::asio::buffer(s));
			} catch (std::exception& error) {
				std::cout << error.what() << std::endl;
			}
			write_mtx_.unlock();
		}
	}

	void websocket_handler::handle_error(error_code error) {
		std::cout << "DevTale server is unavailable, trying to reattach in 1 second..." << std::endl;
		Sleep(1000);
		connect();
	}

	void websocket_handler::rpc_ping(json req, json::value_type id)
	{
		write(jsonrpc::rpc_result("pong", id));
	}

#if DEVTALE_INSECURE

	void websocket_handler::rpc_scan(json req, json::value_type id)
	{
		if (!req.count("params"))
		{
			write(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "params are required for this method", id));
			return;
		}
		const auto params = req["params"].get<std::vector<std::string>>();

		if (params.size() != 2)
		{
			write(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "this method expects 2 params", id));
			return;
		}

		const std::string find = utils::hex_string_to_bytes(params[0]);
		const std::string& mask = params[1];

		const DWORD result = memory::find_pattern((const BYTE*)find.c_str(), mask.c_str());

		write(jsonrpc::rpc_result(utils::dword_to_hex_string(result), id));
	}

	template<typename T> void websocket_handler::rpc_peek(json req, json::value_type id)
	{
		if (!req.count("params"))
		{
			write(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "params are required for this method", id));
			return;
		}
		const auto params = req["params"].get<std::vector<std::string>>();

		if (params.size() != 1)
		{
			write(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "this method expects 1 param", id));
			return;
		}

		const DWORD address = utils::hex_string_to_dword(params[0]);
		const auto result = *reinterpret_cast<T*>(address);

		write(jsonrpc::rpc_result(utils::dword_to_hex_string(result), id));
	}

	template<typename T> void websocket_handler::rpc_poke(json req, json::value_type id)
	{
		if (!req.count("params"))
		{
			write(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "params are required for this method", id));
			return;
		}
		const auto params = req["params"].get<std::vector<std::string>>();

		if (params.size() != 2)
		{
			write(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "this method expects 2 params", id));
			return;
		}

		const DWORD address = utils::hex_string_to_dword(params[0]);
		const T value = static_cast<T>(utils::hex_string_to_dword(params[1]));
		*reinterpret_cast<T*>(address) = value;

		write(jsonrpc::rpc_result(utils::dword_to_hex_string(sizeof(T)), id));
	}

	void websocket_handler::rpc_ntpvm(json req, json::value_type id)
	{
		if (!req.count("params"))
		{
			write(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "params are required for this method", id));
			return;
		}
		const auto params = req["params"].get<std::vector<std::string>>();

		if (params.size() != 3)
		{
			write(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "this method expects 3 params", id));
			return;
		}

		const DWORD dwAddress = utils::hex_string_to_dword(params[0]);
		const DWORD dwSize = utils::hex_string_to_dword(params[1]);
		const DWORD newProtect = utils::hex_string_to_dword(params[2]);
		DWORD oldProtect;

		VirtualProtect(reinterpret_cast<LPVOID>(dwAddress), dwSize, newProtect, &oldProtect);

		write(jsonrpc::rpc_result(utils::dword_to_hex_string(oldProtect), id));
	}

	void websocket_handler::rpc_read(json req, json::value_type id)
	{
		if (!req.count("params"))
		{
			write(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "params are required for this method", id));
			return;
		}
		const auto params = req["params"].get<std::vector<std::string>>();

		if (params.size() != 2)
		{
			write(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "this method expects 2 params", id));
			return;
		}

		const DWORD address = utils::hex_string_to_dword(params[0]);
		const DWORD length = utils::hex_string_to_dword(params[1]);
		std::vector<char> buffer(length + 1);
		memcpy_s(&buffer[0], buffer.size(), reinterpret_cast<void*>(address), length);
		buffer[length] = 0;
		const auto bytes = "0x" + boost::algorithm::hex(std::string(&buffer[0]));

		write(jsonrpc::rpc_result(bytes, id));
	}

	void websocket_handler::rpc_write(json req, json::value_type id)
	{
		if (!req.count("params"))
		{
			write(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "params are required for this method", id));
			return;
		}
		const auto params = req["params"].get<std::vector<std::string>>();

		if (params.size() != 2)
		{
			write(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "this method expects 2 params", id));
			return;
		}

		const DWORD address = utils::hex_string_to_dword(params[0]);
		const auto bytes = utils::hex_string_to_bytes(params[1]);
		memcpy_s(reinterpret_cast<void*>(address), bytes.size(), &bytes[0], bytes.size());

		write(jsonrpc::rpc_result(utils::dword_to_hex_string(bytes.size()), id));
	}

#else
	void websocket_handler::rpc_disabled_method(json req, json::value_type id)
	{
		write(jsonrpc::rpc_error(JRPCERR_METHOD_DISABLED, "this method is not available in a secure build", id));
	}
#endif

	void websocket_handler::rpc_psend(json req, json::value_type id)
	{
		if (!req.count("params"))
		{
			write(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "params are required for this method", id));
			return;
		}
		const auto params = req["params"].get<std::vector<std::string>>();

		if (params.size() != 1)
		{
			write(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "this method expects 1 param", id));
			return;
		}

		protocol::get()->send(params[0]);
		write(jsonrpc::rpc_result(utils::dword_to_hex_string(params[0].size()), id));
	}

	void websocket_handler::rpc_precv(json req, json::value_type id)
	{
		if (!req.count("params"))
		{
			write(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "params are required for this method", id));
			return;
		}
		const auto params = req["params"].get<std::vector<std::string>>();

		if (params.size() != 1)
		{
			write(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "this method expects 1 param", id));
			return;
		}

		protocol::get()->receive(params[0]);
		write(jsonrpc::rpc_result(utils::dword_to_hex_string(params[0].size()), id));
	}

	void websocket_handler::handle_message()
	{
		std::string raw_json = buffers_to_string(read_buffer_.data());
		std::cout << raw_json << std::endl;

		try {
			json req = json::parse(raw_json);
			const auto id = req.count("id") ? req["id"] : json();

			if (req.count("method") != 1)
			{
				write(jsonrpc::rpc_error(JRPCERR_INVALID_REQUEST, "method field is missing", id));
				return;
			}

			const auto method = req["method"].get<std::string>();

			if (message_handler_map.find(method) != message_handler_map.end())
			{
				try {
					message_handler_map.at(method)(req, id);
				}
				catch (const std::exception& e)
				{
					write(jsonrpc::rpc_error(JRPCERR_INTERNAL_ERROR, e.what(), id));
				}
				return;
			}

			write(jsonrpc::rpc_error(JRPCERR_METHOD_NOT_FOUND, "method not found", id));
		}
		catch (json::exception& e)
		{
			write(jsonrpc::rpc_error(JRPCERR_PARSE_ERROR, e.what()));
		}
	}

}