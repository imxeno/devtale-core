#pragma once

#include <string>
#include <nlohmann/json.hpp>

#define JRPCERR_PARSE_ERROR (-32700)
#define JRPCERR_INVALID_REQUEST (-32600)
#define JRPCERR_METHOD_NOT_FOUND (-32601)
#define JRPCERR_INVALID_PARAMS (-32602)
#define JRPCERR_INTERNAL_ERROR (-32603)

namespace devtale {

	namespace jsonrpc {

		using json = nlohmann::json;

		template <typename T> std::string rpc_call(const std::string& method, T param)
		{
			json out = {
				{"jsonrpc", "2.0"},
				{"method", method},
				{"params", param}
			};
			return out.dump(4);
		}
		
		template <typename T> std::string rpc_result(T result, const json::value_type &id)
		{
			json out = {
				{"jsonrpc", "2.0"},
				{"result", result},
				{"id", id}
			};
			return out.dump(4);
		}
		
		std::string inline rpc_error(int error_code, const std::string &message)
		{
			json out = {
				{"jsonrpc", "2.0"},
				{"error", {
					{ "code", error_code },
					{ "message", message }
				}},
				{"id", json()}
			};
			return out.dump(4);
		}

		std::string inline rpc_error(int error_code, const std::string & message, const json::value_type &id)
		{
			json out = {
				{"jsonrpc", "2.0"},
				{"error", {
					{ "code", error_code },
					{ "message", message }
				}},
				{"id", id},
			};
			return out.dump(4);
		}

	}

}