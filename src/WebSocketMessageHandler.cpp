#include "WebSocketMessageHandler.h"

namespace OBS {
namespace HbbTV {
namespace Client {

/**
 * Constructor.
 */
WebSocketMessageHandler::WebSocketMessageHandler()
  : queue_(nullptr)
{
}


/**
 * Destructor.
 */
WebSocketMessageHandler::~WebSocketMessageHandler()
{
  queue_ = nullptr;
}


/**
 * Sets the synchronous blocking queue that may be used by this handler to 
 * interact with the websocket server in a synchronous manner.
 * 
 * @param queue Pointer to the synchronous blocking queue
 */
void WebSocketMessageHandler::SetQueue(SynchronousBlockingQueue<std::string> *queue)
{
  queue_ = queue;
}


} // namespace Client
} // namespace HbbTV
} // namespace OBS
