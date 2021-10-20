#ifndef __JSONRPC_CLIENT_H__
#define __JSONRPC_CLIENT_H__

#include <string>

namespace OBS {
namespace HbbTV {
namespace Client {

class WebSocketClient;
class WebSocketMessageHandler;

/**
 * Implementation of a simple JSONRPC client.
 */
class JsonRpcClient {

public:

  /**
   * Checks if the specified message is an event or a typical response.
   * 
   * @param message The JSONRPC message
   * 
   * @return true if the message is an event, false otherwise
   */
  static bool IsEvent(std::string message);

  /**
   * Constructor.
   *
   * @param host The JSONRPC service endpoint
   * @param port The JSONRPC service port
   * @param protocol The communication protocol
   * @param path The JSONRPC service path
   * @param message_handler The incoming message handler
   */
  JsonRpcClient(std::string host, int port, std::string protocol, std::string path, WebSocketMessageHandler *message_handler);

  /**
   * Destructor.
   */
  ~JsonRpcClient();

  /**
   * Sends a synchronous (blocking) RPC call.
   * 
   * @param method Contains the object and method to be invoked
   * @param params Contains the set of input parameters (optional)
   *
   * @return the JSONRPC-formatted response
   */
  std::string Call(std::string method, std::string params);

  /**
   * Registers this client with the specified event provided by the specified object.
   * 
   * @param object The name of the object that provides the event
   * @param event The event name
   * 
   * @return true in success, false in failure
   */
  bool RegisterWithEvent(std::string object, std::string event);

  /**
   * Unregisters this client with the specified event provided by the specified object.
   * 
   * @param object The name of the object that provides the event
   * @param event The event name
   * 
   * @return true in success, false in failure
   */
  bool UnregisterWithEvent(std::string object, std::string event);

  WebSocketClient *GetWebSocketClient();

private:

  // The websockets client to use for establishing the communication channel with
  // the Thunder nanoservices
  WebSocketClient *web_socket_client_;

}; // class JsonRpcClient

} // Client
} // HbbTV
} // OBS

#endif // __JSONRPC_CLIENT_H__
