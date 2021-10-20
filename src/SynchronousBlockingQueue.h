#ifndef SYNCHRONOUS_BLOCKING_QUEUE_H_
#define SYNCHRONOUS_BLOCKING_QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <string>

namespace OBS {
namespace HbbTV {
namespace Client {

/**
 * Implements the synchronous blocking queue data structure. A synchronous 
 * blocking queue is basically a queue that may have at most one element. The 
 * consumer blocks until that element is made available by the producer. 
 * 
 * The implementation has been based on the STL condition variable mechanism to 
 * enable the desired blocking functionality.
 */
template<class E>
class SynchronousBlockingQueue {

public:

  /**
   * Constructor.
   */
  explicit SynchronousBlockingQueue();
  
  /**
   * Destructor.
   */
  ~SynchronousBlockingQueue();

  /**
   * Blocks and waits until the queue element becomes available.
   * 
   * @return The queue element
   */
  E Take();

  /**
   * Updates the queue element and notifies the consumer that it is now 
   * available for consumption.
   * 
   * @param E The queue element value
   */
  void Offer(E e);

private:

  bool ready_to_unlock_;
  std::mutex mutex_;
  std::condition_variable cv_;
  E element_;
};


} // namespace Client
} // namespace HbbTV
} // namespace OBS

#endif // SYNCHRONOUS_BLOCKING_QUEUE_H_
