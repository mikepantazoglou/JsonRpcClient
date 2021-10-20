#include "WebSocketClient.h"

namespace OBS {
namespace HbbTV {
namespace Client {

WebSocketClient::WebSocketClient(std::string host, int port, std::string protocol, std::string path, WebSocketMessageHandler *message_handler)
{
  OpenWebSocket(host, port, protocol, path, message_handler);
}


WebSocketClient::~WebSocketClient()
{
  CloseWebSocket();
}


std::string WebSocketClient::SendSynchronousRequest(std::string request)
{
  std::string response_message;

  // return empty response if not connected to websocket server
  if (web_socket_->GetConnectState() != WebSocketConnectState::CONNECTED) {
    return response_message;
  }

  // send request asynchronouslsy
  web_socket_->SendAsync(request);
  
  // block and wait for the response to be received
  response_message = web_socket_->GetQueue()->Take();

  return response_message;
}


WebSocket *WebSocketClient::GetWebSocket()
{
  return web_socket_;
}


void WebSocketClient::OpenWebSocket(std::string host, int port, std::string protocol, std::string path, WebSocketMessageHandler *message_handler)
{
  web_socket_ = new WebSocket(host, port, protocol, path);
  web_socket_->SetIncomingMessageHandler(message_handler);
  while (web_socket_->GetConnectState() != WebSocketConnectState::CONNECTED) {
    usleep(100*1000);
  }
}


void WebSocketClient::CloseWebSocket()
{
  if (nullptr != web_socket_) {
    while (web_socket_->HasEnqueuedMessages()) {
      sleep(1);
    }
    delete web_socket_;
    web_socket_ = nullptr;
  }
}


} // namespace Client
} // namespace HbbTV
} // namespace OBS
