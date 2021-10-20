#ifndef __WEBSOCKET_CLIENT_H__
#define __WEBSOCKET_CLIENT_H__

#include "Common/WebSocket.h"
#include "Common/WebSocketMessageHandler.h"

namespace OBS {
namespace HbbTV {
namespace Client {

/**
 * WebSocket-based client for accessing the HbbTV nanoservice.
 */
class WebSocketClient {

public:

  WebSocketClient(std::string host, int port, std::string protocol, std::string path, WebSocketMessageHandler *message_handler);
  virtual ~WebSocketClient();

  std::string SendSynchronousRequest(std::string message);

  WebSocket *GetWebSocket();

private:

  void OpenWebSocket(std::string host, int port, std::string protocol, std::string path, WebSocketMessageHandler *message_handler);
  void CloseWebSocket();

private:

  WebSocket *web_socket_;

};

} // namespace Client
} // namespace HbbTV
} // namespace OBS

#endif // __WEBSOCKET_CLIENT_H__
