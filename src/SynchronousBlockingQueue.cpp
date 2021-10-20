#include "SynchronousBlockingQueue.h"

namespace OBS {
namespace HbbTV {
namespace Client {

/**
 * Constructor.
 */
template<class E>
SynchronousBlockingQueue<E>::SynchronousBlockingQueue()
  : ready_to_unlock_(false)
{
}


/**
 * Destructor.
 */
template<class E>
SynchronousBlockingQueue<E>::~SynchronousBlockingQueue()
{
}


/**
 * Blocks and waits until the queue element becomes available.
 * 
 * @return The queue element
 */
template<class E>
E SynchronousBlockingQueue<E>::Take()
{
  ready_to_unlock_ = false;
  std::unique_lock<std::mutex> lk(mutex_);
  while (!ready_to_unlock_)
    cv_.wait(lk);
  return element_;
}


/**
 * Updates the queue element and notifies the consumer that it is now 
 * available for consumption.
 * 
 * @param E The queue element value
 */
template<class E>
void SynchronousBlockingQueue<E>::Offer(E e)
{
  std::unique_lock<std::mutex> lk(mutex_);
  element_ = e;
  ready_to_unlock_ = true;
  lk.unlock();
  cv_.notify_one();
}


// Explicitly instantiate the template for particular types
template class SynchronousBlockingQueue<std::string>;


} // namespace Client
} // namespace HbbTV
} // namespace OBS
