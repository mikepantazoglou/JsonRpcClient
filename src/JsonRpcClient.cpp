#include "JsonRpcClient.h"
#include "WebSocketClient.h"
#include "WebSocketMessageHandler.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include <memory>
#include <string>
#include <stdexcept>

#define TEMPLATE_REQUEST_WITH_PARAMS "{\"jsonrpc\":\"2.0\", \"id\":\"%d\", \"method\":\"%s\", \"params\":%s}"
#define TEMPLATE_REQUEST_WITHOUT_PARAMS "{\"jsonrpc\":\"2.0\", \"id\":\"%d\", \"method\":\"%s\"}"

namespace OBS {
namespace HbbTV {
namespace Client {


template<typename ... Args>
static std::string string_format( const std::string& format, Args ... args )
{
  int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1;
  if (size_s <= 0) {
    throw std::runtime_error("Error during formatting.");
  }
  char *buf = (char *) malloc(size_s);
  std::snprintf(buf, size_s, format.c_str(), args ...);
  std::string result(buf, buf + size_s - 1);
  free(buf);
  return result;
}


/**
 * Checks if the specified message is an event or a typical response.
 * 
 * @param message The JSONRPC message
 * 
 * @return true if the message is an event, false otherwise
 */
bool JsonRpcClient::IsEvent(std::string message)
{
  json json_msg = json::parse(message);
  json method = json_msg["method"];
  return (!method.empty());
}


/**
 * Constructor.
 *
 * @param host The JSONRPC service endpoint
 * @param port The JSONRPC service port
 * @param protocol The communication protocol
 * @param path The JSONRPC service path
 * @param message_handler The incoming message handler
 */
JsonRpcClient::JsonRpcClient(std::string host, int port, std::string protocol, std::string path, WebSocketMessageHandler *message_handler)
{
  web_socket_client_ = new WebSocketClient(host, port, protocol, path, message_handler);
}


/**
 * Destructor.
 */
JsonRpcClient::~JsonRpcClient()
{
  if (nullptr != web_socket_client_) {
    delete web_socket_client_;
    web_socket_client_ = nullptr;
  }
}


/**
 * Sends a synchronous (blocking) RPC call.
 * 
 * @param method Contains the object and method to be invoked
 * @param params Contains the set of input parameters (optional)
 *
 * @return the JSONRPC-formatted response
 */
std::string JsonRpcClient::Call(std::string method, std::string params)
{
  static int id = 0;
  std::string request;
  std::string response;

  // safety check
  if (nullptr == web_socket_client_) {
    return response;
  }

  if (params.empty()) {
    request = string_format(TEMPLATE_REQUEST_WITHOUT_PARAMS, ++id, method.c_str());
  }
  else {
    request = string_format(TEMPLATE_REQUEST_WITH_PARAMS, ++id, method.c_str(), params.c_str());
  }

  response = web_socket_client_->SendSynchronousRequest(request);
  return response;
}


/**
 * Registers this client with the specified event provided by the specified object.
 * 
 * @param object The name of the object that provides the event
 * @param event The event name
 * 
 * @return true in success, false in failure
 */
bool JsonRpcClient::RegisterWithEvent(std::string object, std::string event)
{
  std::string register_method = object + "." + "register";
  std::string register_params = "{\"event\":\"" + event + "\", \"id\":\"client.events.1\"}";
  
  std::string register_response = Call(register_method, register_params);
  json json_response = json::parse(register_response);
  if (json_response["result"] != 0) {
    return false;
  }
  return true;
}


/**
 * Unregisters this client with the specified event provided by the specified object.
 * 
 * @param object The name of the object that provides the event
 * @param event The event name
 * 
 * @return true in success, false in failure
 */
bool JsonRpcClient::UnregisterWithEvent(std::string object, std::string event)
{
  std::string unregister_method = object + "." + "unregister";
  std::string unregister_params = "{\"event\":\"" + event + "\", \"id\":\"client.events.1\"}";
  
  std::string unregister_response = Call(unregister_method, unregister_params);
  json json_response = json::parse(unregister_response);
  if (json_response["result"] != 0) {
    return false;
  }
  return true;
}


WebSocketClient *JsonRpcClient::GetWebSocketClient()
{
  return web_socket_client_;
}


} // Client
} // HbbTV
} // OBS
