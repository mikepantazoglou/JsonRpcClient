#ifndef WEB_SOCKET_MESSAGE_HANDLER_H_
#define WEB_SOCKET_MESSAGE_HANDLER_H_

#include "SynchronousBlockingQueue.h"

namespace OBS {
namespace HbbTV {
namespace Client {

/**
 * Interface of the WebSocket message handler, which is used to process
 * incoming messages on behalf of the WebSocket.
 */
class WebSocketMessageHandler {

public:

  /**
   * Constructor.
   */
  WebSocketMessageHandler();

  /**
   * Destructor.
   */
  virtual ~WebSocketMessageHandler();

  /**
   * Handles the specified message.
   * 
   * @param message The message
   */
  virtual void Handle(std::string message) = 0;

  /**
   * Sets the synchronous blocking queue that may be used by this handler to 
   * interact with the websocket server in a synchronous manner.
   * 
   * @param queue Pointer to the synchronous blocking queue
   */
  virtual void SetQueue(SynchronousBlockingQueue<std::string> *queue);

protected:

  /**
   * Pointer to the synchronous blocking queue.
   */
  SynchronousBlockingQueue<std::string> *queue_;

};

} // namespace Client
} // namespace HbbTV
} // namespace OBS

#endif // WEB_SOCKET_MESSAGE_HANDLER_H_
