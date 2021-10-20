#include "WebSocket.h"
#include <cstring>

namespace OBS {
namespace HbbTV {
namespace Client {

/**
 * Executed in a separate thread, this function is used for accepting
 * incoming events from the LWS layer.
 *
 * @param wsi
 * @param reason
 * @param user
 * @param in
 * @param len
 * @return
 */
static int OnWebSocketEvent(
    struct lws *wsi,
    enum lws_callback_reasons reason,
    void *user,
    void *in,
    size_t len)
{
  if (NULL == wsi) {
    return -1;
  }

  struct lws_context *context = lws_get_context(wsi);
  WebSocket *web_socket = (WebSocket *) lws_context_user(context);

  switch (reason)
  {
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
    {
      pthread_mutex_lock(&(web_socket->GetMutex()));
      web_socket->SetConnectState(CONNECTED);
      web_socket->SetHandle(wsi);
      pthread_mutex_unlock(&(web_socket->GetMutex()));
      lws_callback_on_writable(wsi);
      break;
    }
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
    {
      pthread_mutex_lock(&(web_socket->GetMutex()));
      web_socket->SetConnectState(DISCONNECTED);
      pthread_mutex_unlock(&(web_socket->GetMutex()));
      break;
    }
    case LWS_CALLBACK_CLOSED:
    {
      pthread_mutex_lock(&(web_socket->GetMutex()));
      web_socket->SetConnectState(DISCONNECTED);
      pthread_mutex_unlock(&(web_socket->GetMutex()));
      break;
    }
    case LWS_CALLBACK_CLIENT_RECEIVE:
    {
      std::string message((char *) in);
      pthread_mutex_lock(&(web_socket->GetMutex()));
      web_socket->GetIncomingMessageHandler()->Handle(message);
      pthread_mutex_unlock(&(web_socket->GetMutex()));
      break;
    }
    case LWS_CALLBACK_CLIENT_WRITEABLE:
    {
      pthread_mutex_lock(&(web_socket->GetMutex()));
      if (web_socket->HasEnqueuedMessages()) {
        std::string message = web_socket->RemoveOldestEnqueuedMessage();
        web_socket->WriteMessageToWebSocket(message);
      }
      else {
        web_socket->SetWritable(true);
      }
      pthread_mutex_unlock(&(web_socket->GetMutex()));
      break;
    }
    default:
      break;
  }

  return 0;
}

/**
 * Executed in a separate thread, this function is used for establishing and
 * maintaining the connection to the WebSocket server.
 *
 * @param arg
 * @return
 */
static void *RunWatchdog(void *arg)
{
  WebSocket *web_socket = (WebSocket *) arg;
  if (NULL == web_socket) {
    return NULL;
  }

  web_socket->SetWatchDogThreadStarted();

  while (web_socket->IsWatchDogThreadRunning())
  {
    if (DISCONNECTED != web_socket->GetConnectState()) {
      lws_service(web_socket->GetContext(), 500);
    }
    else {
      usleep(50*1000);

      struct lws_protocols protocols[] = {
        { web_socket->GetProtocolName().c_str(), OnWebSocketEvent, 32*1024, 32*1024 },
        { NULL, NULL, 0, 0 }
      };

      struct lws_context_creation_info info;
      memset(&info, 0, sizeof(info));
      info.port = CONTEXT_PORT_NO_LISTEN;
      info.protocols = protocols;
      info.gid = -1;
      info.uid = -1;
      info.user = (void *) web_socket;
      lws_context *context = lws_create_context(&info);

      pthread_mutex_lock(&(web_socket->GetMutex()));

      web_socket->SetContext(context);

      struct lws *handle = lws_client_connect(
            web_socket->GetContext(),
            web_socket->GetHost().c_str(),
            web_socket->GetPort(),
            0,
            web_socket->GetServerUrlPath().c_str(),
            web_socket->GetHostAddress().c_str(),
            web_socket->GetHost().c_str(),
            NULL,
            -1);

      if (handle) {
        web_socket->SetConnectState(CONNECTING);
      }

      pthread_mutex_unlock(&(web_socket->GetMutex()));
    }
  }

  lws_cancel_service(web_socket->GetContext());
  if (web_socket->GetContext() != nullptr) {
    lws_context_destroy(web_socket->GetContext());
  }

  pthread_exit(NULL);

  return NULL;
}


/**
 * Constructor.
 * 
 * @param host The websocket server host name
 * @param port The websocket server port
 * @param protocol The websocket server protocol
 * @param path The websocket server path
 */
WebSocket::WebSocket(std::string host, int port, std::string protocol, std::string path)
  : host_(host)
  , port_(port)
  , protocol_name_(protocol)
  , path_(path)
  , context_(nullptr)
  , handle_(nullptr)
  , connection_state_(DISCONNECTED)
  , writable_(false)
  , incoming_message_handler_(nullptr)
  , queue_(new SynchronousBlockingQueue<std::string>())
  , watchdog_thread_running_(false)
{
  pthread_mutex_init(&mutex_, NULL);
  pthread_create(&watchdog_thread_, NULL, &RunWatchdog, (void *) this);
}


/**
 * Destructor.
 */
WebSocket::~WebSocket()
{
  enqueued_messages_.clear();
  StopWatchDogThread();
  pthread_join(watchdog_thread_, NULL);
  incoming_message_handler_ = nullptr;
  handle_ = nullptr;
  context_ = nullptr;
  pthread_mutex_destroy(&mutex_);
  delete queue_;
}


/**
 * Sets the handler of incoming messages.
 * 
 * @param handler Pointer to the incoming message handler
 */
void WebSocket::SetIncomingMessageHandler(WebSocketMessageHandler *handler)
{
  incoming_message_handler_ = handler;
  incoming_message_handler_->SetQueue(queue_);
}


/**
 * Gets the handler of incoming messages.
 * 
 * @return Pointer to the incoming message handler
 */
WebSocketMessageHandler *WebSocket::GetIncomingMessageHandler()
{
  return incoming_message_handler_;
}


/**
 * Sends the given message to the websocket server in an asynchronous manner.
 * If the websocket is writable, the message will be sent immediately.
 * Otherwise, the message will be enqueued and will be sent as soon as the 
 * websocket becomes writable again.
 * 
 * @param message The message to be sent
 */
void WebSocket::SendAsync(std::string message)
{
  pthread_mutex_lock(&mutex_);
  if (writable_) {
    WriteMessageToWebSocket(message);
  }
  else {
    EnqueueMessage(message);
  }
  pthread_mutex_unlock(&mutex_);
}


/**
 * Gets the websocket server host name.
 * 
 * @return The websocket server host name
 */
std::string WebSocket::GetHost() const
{
  return host_;
}


/**
 * Gets the websocket server port.
 * 
 * @return The websocket server port
 */
int WebSocket::GetPort() const
{
  return port_;
}


/**
 * Gets the websocket server address in the form <host>:<port>.
 * 
 * @return The websocket server address
 */
std::string WebSocket::GetHostAddress() const
{
  std::string host_address = host_ + ":" + std::to_string(port_);
  return host_address;
}


/**
 * Gets the websocket server protocol name.
 * 
 * @return The websocket server protocol name
 */
std::string WebSocket::GetProtocolName() const
{
  return protocol_name_;
}


/**
 * Gets the websocket server URL path.
 * 
 * @return The websocket server URL path
 */
std::string WebSocket::GetServerUrlPath() const
{
  return path_;
}


/**
 * Sets the LWS context of this websocket.
 * 
 * @param context Pointer to the created LWS context
 */
void WebSocket::SetContext(lws_context *context)
{
  context_ = context;
}


/**
 * Gets the LWS context of this websocket.
 * 
 * @return Pointer to the LWS context of this websocket
 */
struct lws_context *WebSocket::GetContext()
{
  return context_;
}


/**
 * Sets the LWS handle of this websocket.
 * 
 * @param handle Pointer to the LWS handle
 */
void WebSocket::SetHandle(lws *handle)
{
  handle_ = handle;
}


/**
 * Gets the LWS handle of this websocket.
 * 
 * @return Pointer to the LWS handle of this websocket
 */
struct lws *WebSocket::GetHandle()
{
  return handle_;
}


/**
 * Sets the connection state of this websocket.
 * 
 * @param state The connection state of this websocket
 */
void WebSocket::SetConnectState(WebSocketConnectState state)
{
  connection_state_ = state;
}


/**
 * Gets the connection state of this websocket.
 * 
 * @return The connection state of this websocket
 */
WebSocketConnectState WebSocket::GetConnectState() const
{
  return connection_state_;
}


/**
 * Sets the writable flag of this websocket.
 * 
 * @param writable true/false
 */
void WebSocket::SetWritable(bool writable)
{
  writable_ = writable;
}


/**
 * Gets the writable flag of this websocket.
 * 
 * @return true/false
 */
bool WebSocket::IsWritable() const
{
  return writable_;
}


/**
 * Gets the mutex that this websocket is using to synchronize its threads.
 * 
 * @return Reference to the mutex
 */
pthread_mutex_t& WebSocket::GetMutex()
{
  return mutex_;
}


/**
 * Sends the given message to the websocket server in an asynchronous manner.
 * If the websocket is writable, the message will be sent immediately.
 * Otherwise, the message will be enqueued and will be sent as soon as the 
 * websocket becomes writable again.
 * 
 * @param message The message to be sent
 */
void WebSocket::EnqueueMessage(std::string message)
{
  enqueued_messages_.push_back(message);
}


/**
 * Removes the oldest enqueued message from the message queue.
 * 
 * @return The oldest enqueued message
 */
std::string WebSocket::RemoveOldestEnqueuedMessage()
{
  std::string message;
  if (HasEnqueuedMessages()) {
    message = enqueued_messages_[0];
    enqueued_messages_.erase(enqueued_messages_.begin());
  }
  return message;
}


/**
 * Checks if the internal message queue has content or if it is empty.
 * 
 * @return true/false
 */
bool WebSocket::HasEnqueuedMessages() const
{
  return !enqueued_messages_.empty();
}


/**
 * Gets the synchronization queue of this web socket. The synchronization queue 
 * is used to realise synchronous, blocking calls on behalf of the web socket 
 * client.
 * 
 * @return The synchronous blocking queue of this WebSocket
 */
SynchronousBlockingQueue<std::string> *WebSocket::GetQueue()
{
  return queue_;
}


/**
 * Writes the specified string message to the web socket.
 * 
 * @param message The string message to be written
 */
void WebSocket::WriteMessageToWebSocket(std::string message)
{
  unsigned char *buffer = (unsigned char *) malloc( sizeof(char) *
    (LWS_SEND_BUFFER_PRE_PADDING
      + message.length()
      + LWS_SEND_BUFFER_POST_PADDING)
  );

  memcpy(
    buffer + LWS_SEND_BUFFER_PRE_PADDING,
    message.c_str(),
    message.length()
  );

  lws_write(
    handle_,
    buffer + LWS_SEND_BUFFER_PRE_PADDING,
    message.length(),
    LWS_WRITE_TEXT
  );

  writable_ = 0;
  lws_callback_on_writable(handle_);
  free(buffer);
}


/**
 * Sets the |warchdog_thread_running_| flag to true.
 */
void WebSocket::SetWatchDogThreadStarted()
{
  pthread_mutex_lock(&mutex_);
  watchdog_thread_running_ = true;
  pthread_mutex_unlock(&mutex_);
}


/**
 * Sets the |watchdog_thread_running_| flag to false so as to notify the 
 * watchdog thread that it has to exit.
 */
void WebSocket::StopWatchDogThread()
{
  pthread_mutex_lock(&mutex_);
  watchdog_thread_running_ = false;
  pthread_mutex_unlock(&mutex_);
}


/**
 * Returns true if the watchdog thread is currently running, 
 * otherwise returns false.
 * 
 * @return true/false
 */
bool WebSocket::IsWatchDogThreadRunning() const
{
  return watchdog_thread_running_;
}

} // namespace Client
} // namespace HbbTV
} // namespace OBS
