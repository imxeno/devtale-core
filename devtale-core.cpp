// devtale-core.cpp : Defines the entry point for the application.
//
#include "devtale-core.h"
#include "jsonrpc.hpp"
#include "utils.hpp"
#include "memory.h"
#include "protocol.h"

using namespace devtale;

using tcp = boost::asio::ip::tcp;
using io_context = boost::asio::io_context;
namespace websocket = boost::beast::websocket;
using json = nlohmann::json;

bool running = true;
const auto host = "127.0.0.1";
const auto port = "17171";
const auto ws_target = "/";

typedef void (*message_handler)(websocket::stream<tcp::socket>& ws, json req, json::value_type id);


void rpc_ping(websocket::stream<tcp::socket> &ws, json req, json::value_type id)
{
	ws.write(boost::asio::buffer(jsonrpc::rpc_result("pong", id)));
}

void rpc_scan(websocket::stream<tcp::socket>& ws, json req, json::value_type id)
{
	if(!req.count("params"))
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "params are required for this method", id)));
		return;
	}
	const auto params = req["params"].get<std::vector<std::string>>();
	
	if(params.size() != 2)
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "this method expects 2 params", id)));
		return;
	}
	
	const std::string find = utils::hex_string_to_bytes(params[0]);
	const std::string &mask = params[1];

	const DWORD result = memory::find_pattern((const BYTE*)find.c_str(), mask.c_str());
	
	ws.write(boost::asio::buffer(jsonrpc::rpc_result(utils::dword_to_hex_string(result), id)));
}

template<typename T> void rpc_peek(websocket::stream<tcp::socket>& ws, json req, json::value_type id)
{
	if (!req.count("params"))
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "params are required for this method", id)));
		return;
	}
	const auto params = req["params"].get<std::vector<std::string>>();

	if (params.size() != 1)
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "this method expects 1 param", id)));
		return;
	}
	
	const DWORD address = utils::hex_string_to_dword(params[0]);
	const auto result = *reinterpret_cast<T*>(address);
	
	ws.write(boost::asio::buffer(jsonrpc::rpc_result(utils::dword_to_hex_string(result), id)));
}

template<typename T> void rpc_poke(websocket::stream<tcp::socket>& ws, json req, json::value_type id)
{
	if (!req.count("params"))
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "params are required for this method", id)));
		return;
	}
	const auto params = req["params"].get<std::vector<std::string>>();

	if (params.size() != 2)
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "this method expects 2 params", id)));
		return;
	}

	const DWORD address = utils::hex_string_to_dword(params[0]);
	const T value = utils::hex_string_to_dword(params[1]);
	*reinterpret_cast<T*>(address) = value;

	ws.write(boost::asio::buffer(jsonrpc::rpc_result(utils::dword_to_hex_string(sizeof(T)), id)));
}

void rpc_ntpvm(websocket::stream<tcp::socket>& ws, json req, json::value_type id)
{
	if (!req.count("params"))
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "params are required for this method", id)));
		return;
	}
	const auto params = req["params"].get<std::vector<std::string>>();

	if (params.size() != 3)
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "this method expects 3 params", id)));
		return;
	}

	const DWORD dwAddress = utils::hex_string_to_dword(params[0]);
	const DWORD dwSize = utils::hex_string_to_dword(params[1]);
	const DWORD newProtect = utils::hex_string_to_dword(params[2]);
	DWORD oldProtect;

	VirtualProtect(reinterpret_cast<LPVOID>(dwAddress), dwSize, newProtect, &oldProtect);

	ws.write(boost::asio::buffer(jsonrpc::rpc_result(utils::dword_to_hex_string(oldProtect), id)));
}

void rpc_read(websocket::stream<tcp::socket>& ws, json req, json::value_type id)
{
	if (!req.count("params"))
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "params are required for this method", id)));
		return;
	}
	const auto params = req["params"].get<std::vector<std::string>>();

	if (params.size() != 2)
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "this method expects 2 params", id)));
		return;
	}

	const DWORD address = utils::hex_string_to_dword(params[0]);
	const DWORD length = utils::hex_string_to_dword(params[1]);
	std::vector<char> buffer(length + 1);
	memcpy_s(&buffer[0], buffer.size(), reinterpret_cast<void*>(address), length);
	buffer[length] = 0;
	const auto bytes = "0x" + boost::algorithm::hex(std::string(&buffer[0]));

	ws.write(boost::asio::buffer(jsonrpc::rpc_result(bytes, id)));
}

void rpc_write(websocket::stream<tcp::socket>& ws, json req, json::value_type id)
{
	if (!req.count("params"))
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "params are required for this method", id)));
		return;
	}
	const auto params = req["params"].get<std::vector<std::string>>();

	if (params.size() != 2)
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "this method expects 2 params", id)));
		return;
	}

	const DWORD address = utils::hex_string_to_dword(params[0]);
	const auto bytes = utils::hex_string_to_bytes(params[1]);
	memcpy_s(reinterpret_cast<void*>(address), bytes.size(), &bytes[0], bytes.size());

	ws.write(boost::asio::buffer(jsonrpc::rpc_result(utils::dword_to_hex_string(bytes.size()), id)));
}

void rpc_psend(websocket::stream<tcp::socket>& ws, json req, json::value_type id)
{
	if (!req.count("params"))
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "params are required for this method", id)));
		return;
	}
	const auto params = req["params"].get<std::vector<std::string>>();

	if (params.size() != 1)
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "this method expects 1 param", id)));
		return;
	}

	protocol::get()->send(params[0]);
	ws.write(boost::asio::buffer(jsonrpc::rpc_result(utils::dword_to_hex_string(params[0].size()), id)));
}

void rpc_precv(websocket::stream<tcp::socket>& ws, json req, json::value_type id)
{
	if (!req.count("params"))
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "params are required for this method", id)));
		return;
	}
	const auto params = req["params"].get<std::vector<std::string>>();

	if (params.size() != 1)
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_PARAMS, "this method expects 1 param", id)));
		return;
	}

	protocol::get()->receive(params[0]);
	ws.write(boost::asio::buffer(jsonrpc::rpc_result(utils::dword_to_hex_string(params[0].size()), id)));
}


const std::map<std::string, message_handler> message_handler_map
{
	std::make_pair("ping", &rpc_ping),
	std::make_pair("scan", &rpc_scan),
	std::make_pair("peek", &rpc_peek<BYTE>),
	std::make_pair("peekw", &rpc_peek<WORD>),
	std::make_pair("peekd", &rpc_peek<DWORD>),
	std::make_pair("poke", &rpc_poke<BYTE>),
	std::make_pair("pokew", &rpc_poke<WORD>),
	std::make_pair("poked", &rpc_poke<DWORD>),
	std::make_pair("ntpvm", &rpc_ntpvm),
	std::make_pair("read", &rpc_read),
	std::make_pair("write", &rpc_write),
	std::make_pair("psend", &rpc_psend),
	std::make_pair("precv", &rpc_precv)
};

void handle_message(websocket::stream<tcp::socket>& ws, boost::beast::multi_buffer& buffer)
{
	std::string raw_json = buffers_to_string(buffer.data());
	buffer = boost::beast::multi_buffer();
	std::cout << raw_json << std::endl;

	try {
		json req = json::parse(raw_json);
		const auto id = req.count("id") ? req["id"] : json();

		if (req.count("method") != 1)
		{
			ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INVALID_REQUEST, "method field is missing", id)));
			return;
		}

		const auto method = req["method"].get<std::string>();

		if(message_handler_map.find(method) != message_handler_map.end())
		{
			try {
				message_handler_map.at(method)(ws, req, id);
			}
			catch(const std::exception &e)
			{
				ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_INTERNAL_ERROR, e.what(), id)));
			}
			return;
		}

		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_METHOD_NOT_FOUND, "method not found", id)));
	}
	catch (json::exception& e)
	{
		ws.write(boost::asio::buffer(jsonrpc::rpc_error(JRPCERR_PARSE_ERROR, e.what())));
	}
}

void CreateDebugWindow()
{
	FILE* n_in;
	FILE* n_out;
	FILE* n_err;
	AllocConsole();
	freopen_s(&n_in, "CONIN$", "r", stdin);
	freopen_s(&n_out, "CONOUT$", "w", stdout);
	freopen_s(&n_err, "CONOUT$", "w", stderr);
	std::cout.clear();
}

int main()
{
#ifdef _DEBUG
	CreateDebugWindow();
#endif

	io_context ioc;
	websocket::stream<tcp::socket> ws{ ioc };
	tcp::resolver resolver{ ioc };
	packet_handler phandler(ws);
	protocol::get()->set_packet_handler(&phandler);
	
	auto const results = resolver.resolve(host, port);
	while (running) {
		try {
			boost::asio::connect(ws.next_layer(), results.begin(), results.end());
			ws.handshake(host, ws_target);

			boost::beast::multi_buffer buffer;
			while (ws.read(buffer))
			{
				handle_message(ws, buffer);
			}

			ws.close(websocket::close_code::normal);
		}
		catch (boost::system::system_error&)
		{
			std::cout << "DevTale server is unavailable, trying to reattach in 1 second..." << std::endl;
			Sleep(1000);
		}
	}
}

// ReSharper disable once CppInconsistentNaming
bool WINAPI DllMain(_In_ HINSTANCE instance, _In_ DWORD call_reason, _In_ LPVOID reserved)
{
	UNUSED(instance);
	UNUSED(reserved);
	
	switch (call_reason)
	{
	case DLL_PROCESS_ATTACH:
		{
			std::thread main_thread(main);
			main_thread.detach();
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
	default:
		break;
	}
	return TRUE;
}
