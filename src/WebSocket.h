#ifndef WEB_SOCKET_H_
#define WEB_SOCKET_H_

#include <libwebsockets.h>
#include <pthread.h>
#include <string>
#include <vector>

#include "SynchronousBlockingQueue.h"
#include "WebSocketMessageHandler.h"

namespace OBS {
namespace HbbTV {
namespace Client {

/**
 * Enumerates the WebSocket connection state.
 */
enum WebSocketConnectState {
  DISCONNECTED = 0,
  CONNECTING,
  CONNECTED
};

/**
 * Simple implementation of a WebSocket client that can be used for
 * sending and receiving string messages to/from a WebSocket server.
 * The implementation is based on the LibWebSockets (LWS) framework.
 * (https://libwebsockets.org/, v1.6.2)
 * 
 * The WebSocket client uses three separate threads as follows:
 * 
 * (1) The watchdog thread is used to establish, maintain, and close the 
 *     connection to the websocket server.
 * (2) The main thread is used by the user to construct the websocket client 
 *     and subsequently send messages to the websocket server.
 * (3) Another thread is used by the LWS layer to respond to LWS events.
 * 
 * So, in short, the WebSocket offers the following functionality:
 * 
 * - Run watchdog to keep the websocket client connected to the server at all times
 * - Send asynchronous JSON messages to the websocket server
 * - Ensure outgoing JSON messages are sent to the websocket server in the correct order
 * - Respond to primitive LWS events and maintain websocket connectivity state
 * - Pass incoming JSON messages to the upper layer
 * - Clear LWS resources upon destruction
 */
class WebSocket {

public:

  /**
   * Constructor.
   * 
   * @param host The websocket server host name
   * @param port The websocket server port
   * @param protocol The websocket server protocol
   * @param path The websocket server path
   */ 
  explicit WebSocket(std::string host, int port, std::string protocol, std::string path);

  /**
   * Destructor.
   */
  ~WebSocket();

  /**
   * Sets the handler of incoming messages.
   * 
   * @param handler Pointer to the incoming message handler
   */
  void SetIncomingMessageHandler(WebSocketMessageHandler *handler);
  
  /**
   * Gets the handler of incoming messages.
   * 
   * @return Pointer to the incoming message handler
   */
  WebSocketMessageHandler *GetIncomingMessageHandler();

  /**
   * Sends the given message to the websocket server in an asynchronous manner.
   * If the websocket is writable, the message will be sent immediately.
   * Otherwise, the message will be enqueued and will be sent as soon as the 
   * websocket becomes writable again.
   * 
   * @param message The message to be sent
   */
  void SendAsync(std::string message);

  /**
   * Gets the websocket server host name.
   * 
   * @return The websocket server host name
   */
  std::string GetHost() const;

  /**
   * Gets the websocket server port.
   * 
   * @return The websocket server port
   */
  int GetPort() const;

  /**
   * Gets the websocket server address in the form <host>:<port>.
   * 
   * @return The websocket server address
   */
  std::string GetHostAddress() const;

  /**
   * Gets the websocket server protocol name.
   * 
   * @return The websocket server protocol name
   */
  std::string GetProtocolName() const;

  /**
   * Gets the websocket server URL path.
   * 
   * @return The websocket server URL path
   */
  std::string GetServerUrlPath() const;

  /**
   * Sets the LWS context of this websocket.
   * 
   * @param context Pointer to the created LWS context
   */
  void SetContext(lws_context *context);

  /**
   * Gets the LWS context of this websocket.
   * 
   * @return Pointer to the LWS context of this websocket
   */
  struct lws_context *GetContext();

  /**
   * Sets the LWS handle of this websocket.
   * 
   * @param handle Pointer to the LWS handle
   */
  void SetHandle(struct lws *handle);

  /**
   * Gets the LWS handle of this websocket.
   * 
   * @return Pointer to the LWS handle of this websocket
   */
  struct lws *GetHandle();

  /**
   * Sets the connection state of this websocket.
   * 
   * @param state The connection state of this websocket
   */
  void SetConnectState(WebSocketConnectState state);

  /**
   * Gets the connection state of this websocket.
   * 
   * @return The connection state of this websocket
   */
  WebSocketConnectState GetConnectState() const;

  /**
   * Sets the writable flag of this websocket.
   * 
   * @param writable true/false
   */
  void SetWritable(bool writable);
  
  /**
   * Gets the writable flag of this websocket.
   * 
   * @return true/false
   */
  bool IsWritable() const;

  /**
   * Gets the mutex that this websocket is using to synchronize its threads.
   * 
   * @return Reference to the mutex
   */
  pthread_mutex_t& GetMutex();

  /**
   * Enqueues the given message so that it will be sent to the websocket server 
   * at a later stage, as soon as the websocket becomes writable.
   * 
   * @param message The message to be enqueued
   */
  void EnqueueMessage(std::string message);

  /**
   * Removes the oldest enqueued message from the internal message queue.
   * 
   * @return The oldest enqueued message
   */
  std::string RemoveOldestEnqueuedMessage();

  /**
   * Checks if the internal message queue has content or if it is empty.
   * 
   * @return true/false
   */
  bool HasEnqueuedMessages() const;

  /**
   * Gets the synchronization queue of this web socket. The synchronization queue 
   * is used to realise synchronous, blocking calls on behalf of the web socket 
   * client.
   * 
   * @return The synchronous blocking queue of this WebSocket
   */
  SynchronousBlockingQueue<std::string> *GetQueue();

  /**
   * Writes the specified string message to the web socket.
   * 
   * @param message The string message to be written
   */
  void WriteMessageToWebSocket(std::string message);

  /**
   * Sets the |warchdog_thread_running_| flag to true.
   */
  void SetWatchDogThreadStarted();
  
  /**
   * Sets the |watchdog_thread_running_| flag to false so as to notify the 
   * watchdog thread that it has to exit.
   */
  void StopWatchDogThread();
  
  /**
   * Returns true if the watchdog thread is currently running, 
   * otherwise returns false.
   * 
   * @return true/false
   */
  bool IsWatchDogThreadRunning() const;

private:

  // member variables

  /**
   * The websocket server host name.
   */
  std::string host_;
  
  /**
   * The websocket server port.
   */
  int port_;
  
  /**
   * The websocket server protocol name.
   */
  std::string protocol_name_;

  /**
   * The websocket server URL path.
   */
  std::string path_;
  
  /**
   * Pointer to the LWS context.
   */
  struct lws_context *context_;
  
  /**
   * Pointer to the LWS handle.
   */
  struct lws *handle_;

  /**
   * The websocket connection state.
   */
  WebSocketConnectState connection_state_;
  
  /**
   * Flag that indicates whether this websocket is writable or not.
   */
  bool writable_;

  /**
   * The watchdog thread.
   */
  pthread_t watchdog_thread_;

  /**
   * The mutex used to synchronize the websocket threads.
   */
  pthread_mutex_t mutex_;

  /**
   * The list of enqueued messages.
   */
  std::vector<std::string> enqueued_messages_;
  
  /**
   * Pointer to the incoming message handler.
   */
  WebSocketMessageHandler *incoming_message_handler_;

  /**
   * Pointer to the synchronous blocking queue that may be used by upper layers 
   * to implement synchronous interactions with the websocket server.
   */
  SynchronousBlockingQueue<std::string> *queue_;
  
  /**
   * Flag that indicates whether the watchdog thread should be running or not.
   */
  bool watchdog_thread_running_;
};

} // namespace Client
} // namespace HbbTV
} // namespace OBS

#endif // WEB_SOCKET_H_
