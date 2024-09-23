
#include <pthread.h>
#include <errno.h>
#include <seahorn/seahorn.h>
#include "nondet.h"

int nd_error() {
  return nd_int();
}

pthread_t g_thread_1 = 0;

/**
 * @brief Creates a new thread.
 *
 * This function starts a new thread in the calling process. The new thread
 * starts execution by invoking start_routine; arg is passed as the sole
 * argument of start_routine.
 *
 * @param thread Pointer to the pthread_t variable that receives the thread ID
 * of the new thread.
 * @param attr A pointer to a pthread_attr_t structure that specifies the
 * attributes for the new thread; if NULL, the default attributes are used.
 * @param start_routine Pointer to the function that the new thread will execute
 * once it is created.
 * @param arg A single argument that may be passed to start_routine. It must be
 * passed by reference as a pointer cast of type void*.
 * @return int Returns 0 on successful thread creation, otherwise a non-zero
 * error number is returned.
 *
 * @note The new thread terminates in one of three ways:
 *       - It calls pthread_exit(), specifying an exit status value that is
 * available to another thread in the same process that calls pthread_join().
 *       - It returns from start_routine(). This is equivalent to calling
 * pthread_exit() with the value that start_routine() returns.
 *       - It is canceled (see pthread_cancel).
 *       - Ensure that resources are properly freed and shared data is handled
 * correctly when using threads.
 */
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, 
                   void *(*start_routine) (void *), void *arg) {
  if (nd_bool()) {
    return nd_error();
  }
  sassert(sea_is_dereferenceable(thread, sizeof(pthread_t)));
  if (!attr) {
    return EINVAL;
  }
  sassert(sea_is_dereferenceable(attr, sizeof(pthread_attr_t)));
  sassert(sea_is_dereferenceable(start_routine, sizeof(start_routine)));
  *thread = g_thread_1;
  start_routine(arg);
  return 0;
}

/**
 * @brief Waits for the thread specified by thread to terminate.
 *
 * If that thread has already terminated, then pthread_join() returns
 * immediately. The thread specified by thread must be joinable.
 *
 * @param thread ID of the thread to wait for.
 * @param retval If not NULL, the function will store the thread's exit value in
 * *retval.
 * @return On success, returns 0; on error, it returns an error number.
 *
 * @note After a successful call to pthread_join(), the caller is responsible
 * for deallocating any dynamic resources used by the thread.
 * @warning The behavior is undefined if the value specified by the thread
 * argument to pthread_join() does not refer to a joinable thread.
 */
int pthread_join(pthread_t thread, void **retval) {
  if (nd_bool()) {
    return nd_error();
  }
  if (thread != g_thread_1) {
    return ESRCH;
  }
  return 0;
}

/**
 * @brief Retrieves the unique identifier of the current thread.
 *
 * This function is used to get the thread ID of the calling thread.
 * This can be useful for comparing the current thread ID with other thread IDs
 * or for using the thread ID as a key in a data structure.
 *
 * @return This function returns the thread ID of the calling thread.
 *
 * Example usage:
 * @code
 * pthread_t tid = pthread_self();
 * if (pthread_equal(tid, main_tid)) {
 *     // This is the main thread
 * }
 * @endcode
 *
 * @note The pthread_self() function is always successful and no return value is
 * reserved to indicate an error.
 *
 * @see pthread_create, pthread_equal
 */
pthread_t pthread_self(void) {
  // This function always succeeds.
  return g_thread_1;
}

/**
 * @brief Compares two thread identifiers.
 *
 * The pthread_equal() function compares two thread identifiers t1 and t2.
 * 
 * @param t1 The first thread identifier to compare.
 * @param t2 The second thread identifier to compare.
 *
 * @return This function returns a non-zero value if t1 and t2 are equal, otherwise, it returns 0.
 *
 * @note The function can be used to determine if two thread identifiers refer to the same thread.
 * 
 * Example Usage:
 * @code
 * pthread_t tid1, tid2;
 * // ... create or get thread IDs for tid1 and tid2
 * if (pthread_equal(tid1, tid2)) {
 *     printf("Threads are the same.\n");
 * } else {
 *     printf("Threads are different.\n");
 * }
 * @endcode
 */
int pthread_equal(pthread_t t1, pthread_t t2) {
  // This function always succeeds.
  return t1 == t2;
}

/**
 * @brief Detaches the specified thread.
 *
 * This function detaches the specified thread. Once a thread is detached,
 * its resources are automatically released back to the system upon termination
 * without the need for another thread to join with it.
 *
 * @param thread The thread to be detached.
 * @return On success, returns 0; on error, returns an error number.
 *
 * @note After a successful call to pthread_detach(), the thread becomes
 * unjoinable and the thread ID becomes invalid.
 */
int pthread_detach(pthread_t thread) {
  if (nd_bool()) {
    return nd_error();
  }
  if (thread != g_thread_1) {
    return ESRCH;
  }
  return 0;
}

/**
 * @brief Ensure that a specific initialization routine is called only once.
 *
 * The pthread_once function is a synchronization primitive that ensures
 * a specified initialization routine is called only once, regardless of
 * how many threads call it. It's useful for initializing resources in
 * multi-threaded programs.
 *
 * @param once_control A pointer to a pthread_once_t object that determines
 *                     whether the initialization routine has been called.
 *                     This object must be statically initialized with
 *                     PTHREAD_ONCE_INIT.
 * @param init_routine The initialization routine to be called. This pointer
 *                     to a function takes no arguments and returns no value.
 *
 * @return On success, pthread_once returns 0. If an error occurs, it returns
 *         an error number.
 *
 * @note If init_routine is called, it will complete before pthread_once
 * returns.
 */
int pthread_once(pthread_once_t *once_control, void (*init_routine)(void)) {
  if (nd_bool()) {
    return nd_error();
  }
  sassert(sea_is_dereferenceable(once_control, sizeof(pthread_once_t)));
  sassert(sea_is_dereferenceable(init_routine, sizeof(init_routine)));
  init_routine();
  return 0;
}

/**
 * @brief Initializes the thread attributes object.
 *
 * This function initializes the thread attributes object pointed to by attr 
 * with default attribute values. The attr object can then be used in 
 * subsequent calls to pthread_create. Once initialized, the attribute object 
 * can be modified using other pthread attribute functions.
 *
 * @param attr Pointer to an uninitialized pthread attribute object.
 * @return On success, this function returns 0. On error, a non-zero error 
 *         number is returned.
 *
 * @note After you are done with the attribute object, you should destroy 
 *       it using pthread_attr_destroy to free any resources it may hold.
 *
 * Example Usage:
 * @code
 * pthread_attr_t attr;
 * int s = pthread_attr_init(&attr);
 * // Check for success and use the attr object...
 * pthread_attr_destroy(&attr);
 * @endcode
 *
 * @see pthread_attr_destroy, pthread_create
 */
int pthread_attr_init(pthread_attr_t *attr) {
  sassert(sea_is_dereferenceable(attr, sizeof(pthread_attr_t)));
  memhavoc(attr, sizeof(pthread_attr_t)); // FIXME: not sure how to initialize this
  // on Linux these functions always succeed 
  return 0;
}

/**
 * @brief Destroys a thread attributes object
 *
 * The pthread_attr_destroy() function shall destroy a thread attributes object;
 * the object becomes, in effect, uninitialized. An attributes object in this
 * state shall be reinitialized using pthread_attr_init() before reuse.
 *
 * @param attr Reference to the thread attribute object to be destroyed.
 *
 * @return On success, this function returns 0; on error, it returns a nonzero
 * error number.
 *
 * @note After calling pthread_attr_destroy(), the attr object should not be
 * used until it is reinitialized using pthread_attr_init().
 * @warning If the attr parameter is invalid, the behavior of this function is
 * undefined.
 */
int pthread_attr_destroy(pthread_attr_t *attr) {
  sassert(sea_is_dereferenceable(attr, sizeof(pthread_attr_t)));
  memhavoc(attr, sizeof(pthread_attr_t));
  return 0;
}

/**
 * @brief Set the stack size attribute in a thread attributes object.
 * 
 * The pthread_attr_setstacksize() function sets the stack size attribute of
 * the thread attributes object referred to by attr. This attribute specifies
 * the size (in bytes) of the stack that should be allocated for threads that
 * are created using this thread attributes object.
 * 
 * @param attr A pointer to the thread attributes object.
 * @param stacksize The size of the stack to be used by threads.
 * 
 * @return On success, this function returns 0; on error, it returns a nonzero
 * error number.
 * 
 * @note Minimum stack size is PTHREAD_STACK_MIN. Setting stacksize less than
 * this minimum will result in an error. The stack size should be a multiple of
 * the system page size.
 * 
 * @see pthread_attr_getstacksize()
 * 
 * @example
 * pthread_attr_t attr;
 * pthread_attr_init(&attr);
 * pthread_attr_setstacksize(&attr, 1024 * 1024); // Set stack size to 1 MB
 */
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize) {
  if (stacksize < PTHREAD_STACK_MIN) {
    return EINVAL;
  }
  sassert(sea_is_dereferenceable(attr, sizeof(pthread_attr_t)));
  // stack size does not effect verification, treat as noop;
  return 0;
}

/**
 * @brief Locks the specified mutex.
 * 
 * If the mutex is already locked by another thread, the calling thread
 * will block until the mutex becomes available. This function is used
 * to avoid race conditions by synchronizing access to shared resources.
 *
 * @param mutex A pointer to the mutex to be locked. This mutex must be
 * initialized before being used.
 *
 * @return On success, returns 0. If an error occurs, a non-zero error code
 * is returned. Common error codes include EINVAL, indicating that the mutex
 * was not properly initialized, and EDEADLK, indicating that the current
 * thread already owns the mutex (if error checking is enabled for the mutex).
 */
int pthread_mutex_lock(pthread_mutex_t *mutex) {
  if (nd_bool()) {
    return nd_error();
  }
  sassert(sea_is_dereferenceable(mutex, sizeof(pthread_mutex_t)));
  return 0;
}


/**
 * @brief Unlocks the mutex.
 *
 * This function unlocks the mutex specified by 'mutex'. If the mutex is currently
 * locked by the calling thread, the mutex becomes unlocked. If the mutex is not
 * locked or is locked by a different thread, the behavior is undefined.
 *
 * @param mutex A pointer to the mutex to be unlocked.
 * @return On success, returns 0; on error, returns an error number.
 *
 * @note After unlocking the mutex, other threads blocked on this mutex will
 * be able to acquire the mutex. 
 * 
 * @warning The behavior is undefined if 'mutex' is not a valid mutex,
 * or if it is not locked by the calling thread.
 *
 * Example usage:
 * @code
 * pthread_mutex_t mutex;
 * pthread_mutex_init(&mutex, NULL);
 * pthread_mutex_lock(&mutex);
 * // ... [critical section code] ...
 * pthread_mutex_unlock(&mutex);
 * @endcode
 */
int pthread_mutex_unlock(pthread_mutex_t *mutex) {
  if (nd_bool()) {
    return nd_error();
  }
  sassert(sea_is_dereferenceable(mutex, sizeof(pthread_mutex_t)));
  return 0;
}

/**
 * @brief Attempts to lock a mutex without blocking.
 *
 * This function attempts to lock the specified mutex. If the mutex is 
 * already locked by another thread, the function returns immediately 
 * with an error code (instead of blocking the calling thread).
 *
 * @param mutex A pointer to the mutex to be attempted to lock.
 * @return On success, returns 0; on error, returns an error number.
 *         Possible error numbers include:
 *         - EBUSY: The mutex could not be acquired because it was already locked.
 *         - EINVAL: The mutex was not correctly initialized.
 *         - EAGAIN: The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded.
 *         - EDEADLK: A deadlock condition was detected or the current thread already owns the mutex.
 */
int pthread_mutex_trylock(pthread_mutex_t *mutex) {
  if (nd_bool()) {
    return nd_error();
  }
  sassert(sea_is_dereferenceable(mutex, sizeof(pthread_mutex_t)));
  return 0;
}

/**
 * @brief Initializes a mutex.
 *
 * This function initializes the mutex referenced by 'mutex' with attributes specified
 * by 'attr'. If 'attr' is NULL, the default mutex attributes are used.
 *
 * @param mutex A pointer to the mutex to be initialized.
 * @param attr A pointer to a pthread_mutexattr_t structure that specifies the attributes
 *             for the mutex. This value can be NULL for default attributes.
 *
 * @return On success, returns 0; on error, returns an error number.
 *
 * @note If the mutex is already initialized, reinitializing it with this function may 
 *       result in undefined behavior.
 * @note After a successful call to pthread_mutex_init, the state of the mutex becomes 
 *       initialized and unlocked.
 *
 * Example usage:
 * \code{.c}
 * pthread_mutex_t mutex;
 * pthread_mutex_init(&mutex, NULL);
 * \endcode
 */
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
  if (nd_bool()) {
    return nd_error();
  }
  if (attr) {
    sassert(sea_is_dereferenceable(attr, sizeof(pthread_mutexattr_t)));
  }
  sassert(sea_is_dereferenceable(mutex, sizeof(pthread_mutex_t)));
  return 0;
}

/**
 * @brief Destroys the specified mutex.
 *
 * @details This function destroys the mutex object referenced by 'mutex'.
 *          If the mutex is currently locked, the behavior is undefined.
 *          A destroyed mutex object can be reinitialized using pthread_mutex_init(); 
 *          the results of otherwise referencing the object after it has been destroyed are undefined.
 *
 * @param mutex A pointer to the mutex to be destroyed.
 *
 * @return On success, this function returns 0. 
 *         On error, an error number is returned to indicate the error.
 *
 * @note Attempting to destroy a locked mutex results in undefined behavior.
 *
 * @warning It is safe to destroy an initialized mutex that is unlocked. 
 *          Destroying a locked mutex results in undefined behavior.
 */
int pthread_mutex_destroy(pthread_mutex_t *mutex) {
  if (nd_bool()) {
    return nd_error();
  }
  sassert(sea_is_dereferenceable(mutex, sizeof(pthread_mutex_t)));
  return 0;
}

/**
 * @brief Initialize a mutex attributes object
 * 
 * The pthread_mutexattr_init() function shall initialize a mutex attributes object 
 * with the default value for all of the attributes defined by the implementation.
 *
 * @param attr A pointer to the mutex attributes object to be initialized.
 * 
 * @return On success, this function returns 0. If an error occurs, a nonzero error 
 * number is returned. Possible error numbers include but are not limited to EINVAL 
 * if the value specified by attr is invalid.
 *
 * @note After a mutex attributes object has been used to initialize one or more 
 * mutexes, any modification to the attributes object does not affect any previously 
 * initialized mutexes.
 */
int pthread_mutexattr_init(pthread_mutexattr_t *attr) {
  if (nd_bool()) {
    return nd_error();
  }
  sassert(sea_is_dereferenceable(attr, sizeof(pthread_mutexattr_t)));
  memhavoc(attr, sizeof(pthread_mutexattr_t)); // FIXME: not sure how to initialize this
  // on Linux these functions always succeed 
  return 0;
}

/**
 * @brief Destroys a mutex attribute object.
 *
 * The pthread_mutexattr_destroy() function shall destroy a mutex attributes object; 
 * the object becomes, in effect, uninitialized. An implementation may cause 
 * pthread_mutexattr_destroy() to set the object referenced by attr to an 
 * invalid value.
 *
 * @param attr Pointer to the mutex attribute object to be destroyed.
 * 
 * @return On success, this function returns 0; on error, it returns an error number.
 * 
 * @note After destroying a mutex attribute object, it should not be reused 
 * until it is reinitialized using pthread_mutexattr_init(). Attempting to 
 * destroy an already destroyed attribute object results in undefined behavior.
 *
 * @example
 * pthread_mutexattr_t attr;
 * pthread_mutexattr_init(&attr);
 * // Set attributes...
 * pthread_mutexattr_destroy(&attr);
 */
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr) {
  if (nd_bool()) {
    return nd_error();
  }
  sassert(sea_is_dereferenceable(attr, sizeof(pthread_mutexattr_t)));
  memhavoc(attr, sizeof(pthread_mutexattr_t));
  return 0;
}

/**
 * @brief Set the mutex type attribute.
 *
 * This function sets the mutex type attribute in a mutex attributes object.
 * The type of a mutex determines its behavior when it is locked multiple 
 * times by the same thread.
 *
 * @param attr A pointer to the mutex attributes object.
 * @param type The type of the mutex. Possible types are 
 *             PTHREAD_MUTEX_NORMAL, PTHREAD_MUTEX_ERRORCHECK, 
 *             PTHREAD_MUTEX_RECURSIVE, and PTHREAD_MUTEX_DEFAULT.
 * 
 * @return On success, this function returns 0. On error, it returns a nonzero 
 *         error number.
 *
 * @note The behavior of a mutex depends on its type attribute. For instance,
 *       PTHREAD_MUTEX_RECURSIVE allows the same thread to lock the mutex 
 *       multiple times without causing deadlock.
 *
 * Example usage:
 * @code
 * pthread_mutexattr_t attr;
 * pthread_mutexattr_init(&attr);
 * pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
 * @endcode
 */
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type) {
  if (nd_bool()) {
    return nd_error();
  }
  sassert(sea_is_dereferenceable(attr, sizeof(pthread_mutexattr_t)));
  return 0;
}

/**
 * @brief Initialize a condition variable.
 *
 * This function initializes the condition variable pointed to by @p cond
 * according to the attributes specified in @p attr. If @p attr is NULL, the
 * default attributes are used.
 *
 * @param cond A pointer to the condition variable to be initialized.
 * @param attr A pointer to a pthread_condattr_t structure that specifies the
 *             attributes for the condition variable. If this parameter is NULL,
 *             default attributes are used.
 *
 * @return On success, this function returns 0. If an error occurs, a nonzero
 *         error code is returned.
 *
 * @note If the condition variable has already been initialized, reinitializing
 *       it with this function may lead to undefined behavior.
 * @note The behavior is undefined if @p cond is an invalid pointer.
 * @note The pthread_cond_init() function shall fail if:
 *       - EAGAIN: The system lacked the necessary resources to initialize
 *         another condition variable.
 *       - ENOMEM: Insufficient memory exists to initialize the condition
 *         variable.
 *       - EBUSY: The implementation has detected an attempt to reinitialize
 *         the object referenced by @p cond, a previously initialized, but not
 *         yet destroyed, condition variable.
 *       - EINVAL: The value specified by @p attr is invalid.
 *
 * Example usage:
 * @code
 * pthread_cond_t cond;
 * pthread_cond_init(&cond, NULL);
 * @endcode
 */
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
  if (nd_bool()) {
    return nd_error();
  }
  if (attr) {
    sassert(sea_is_dereferenceable(attr, sizeof(pthread_condattr_t)));
  }
  sassert(sea_is_dereferenceable(cond, sizeof(pthread_cond_t)));
  return 0;
}

/**
 * @brief Wait on a condition.
 *
 * The pthread_cond_wait() function is used to atomically release the mutex and
 * cause the calling thread to block on the condition variable. The thread
 * unblocks only after another thread calls pthread_cond_signal() or
 * pthread_cond_broadcast(), and the mutex becomes available.
 *
 * @param cond A condition variable on which to wait.
 * @param mutex A mutex. It must be locked by the calling thread on entrance to
 * pthread_cond_wait.
 *
 * @return On success, this function returns 0. On error, it returns a nonzero
 * error number.
 *
 * @note If the mutex was not locked by the calling thread, the behavior is
 * undefined.
 * @note The calling thread re-acquires the mutex before pthread_cond_wait()
 * returns.
 *
 * Example usage:
 * @code
 * pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
 * pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
 * pthread_mutex_lock(&mutex);
 * while (condition_is_not_met())
 *     pthread_cond_wait(&cond, &mutex);
 * pthread_mutex_unlock(&mutex);
 * @endcode
 */
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
  if (nd_bool()) {
    return nd_error();
  }
  sassert(sea_is_dereferenceable(cond, sizeof(pthread_cond_t)));
  sassert(sea_is_dereferenceable(mutex, sizeof(pthread_mutex_t)));
  return 0;
}

/**
 * @brief Waits for a condition variable up to a specified time limit.
 *
 * The pthread_cond_timedwait() function atomically unlocks the mutex 
 * (specified by @p mutex) and waits for the condition variable 
 * (specified by @p cond) to be signaled or broadcast, until the specified
 * time (specified by @p abstime). The thread execution is blocked until 
 * either of these events occurs. Upon successful return, the mutex is 
 * locked again and owned by the calling thread.
 *
 * @param cond A pointer to the condition variable.
 * @param mutex A pointer to the mutex.
 * @param abstime The absolute time until which the wait is performed. 
 *                The time is specified as per the system's clock.
 * 
 * @return On success, returns 0; on error, it returns a nonzero error 
 *         number. Possible error codes are ETIMEDOUT, EINVAL, and others 
 *         as specified by the implementation.
 *
 * @note The mutex must be locked by the calling thread on entrance to 
 *       pthread_cond_timedwait(). If successful, the mutex will be 
 *       locked upon exit.
 *
 * Example usage:
 * @code
 * pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
 * pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
 * struct timespec ts;
 * // Setup ts to current time + desired wait duration
 * pthread_mutex_lock(&mutex);
 * int res = pthread_cond_timedwait(&cond, &mutex, &ts);
 * pthread_mutex_unlock(&mutex);
 * @endcode
 */
int pthread_cond_timedwait(pthread_cond_t *restrict cond,
                           pthread_mutex_t *restrict mutex,
                           const struct timespec *restrict abstime) {
  if (nd_bool()) {
    return nd_error();
  }
  sassert(sea_is_dereferenceable(cond, sizeof(pthread_cond_t)));
  sassert(sea_is_dereferenceable(mutex, sizeof(pthread_mutex_t)));
  sassert(sea_is_dereferenceable(abstime, sizeof(struct timespec)));
  return 0;
}

/**
 * @brief Destroys a condition variable.
 *
 * This function destroys the specified condition variable. The condition
 * variable should not be destroyed while it is being waited on by other
 * threads. Once the condition variable is destroyed, it should not be used
 * unless it is re-initialized using pthread_cond_init().
 *
 * @param cond Pointer to the condition variable to be destroyed.
 *
 * @return On success, returns 0. If an error occurs, a non-zero error
 *         number is returned.
 *
 * @retval 0 Success.
 * @retval EINVAL The value specified by cond is invalid.
 * @retval EBUSY The condition variable is being used by another thread.
 *
 * Example usage:
 * @code
 * pthread_cond_t cond;
 * pthread_cond_init(&cond, NULL);
 * // ... use the condition variable ...
 * pthread_cond_destroy(&cond);
 * @endcode
 *
 * @note It is safe to re-initialize a destroyed condition variable using
 *       pthread_cond_init() if no threads are waiting on it.
 *
 * @see pthread_cond_init, pthread_cond_wait, pthread_cond_signal
 */
int pthread_cond_destroy(pthread_cond_t *cond) {
  if (nd_bool()) {
    return nd_error();
  }
  sassert(sea_is_dereferenceable(cond, sizeof(pthread_cond_t)));
  memhavoc(cond, sizeof(pthread_cond_t));
  return 0;
}

/**
 * @brief Signal a condition variable.
 *
 * The pthread_cond_signal() function is used to unblock at least one of the 
 * threads that are blocked on the specified condition variable cond (if any 
 * threads are blocked on cond).
 *
 * @param cond A pointer to the condition variable to signal. The condition 
 *             variable must be initialized before use.
 *
 * @return On success, this function returns 0. If an error occurs, a nonzero 
 *         error code is returned.
 *
 * @note pthread_cond_signal() may be called by a thread whether or not it 
 *       currently owns the mutex that threads calling pthread_cond_wait() or 
 *       pthread_cond_timedwait() have associated with the condition variable 
 *       during their waits; however, if predictable scheduling behavior is 
 *       required, then that mutex shall be locked by the thread calling 
 *       pthread_cond_signal().
 *
 * Example usage:
 * @code
 * pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
 * pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
 *
 * pthread_mutex_lock(&mutex);
 * // Perform some action that changes a condition making a signal necessary.
 * pthread_cond_signal(&cond);
 * pthread_mutex_unlock(&mutex);
 * @endcode
 */
int pthread_cond_signal(pthread_cond_t *cond) {
  if (nd_bool()) {
    return nd_error();
  }
  sassert(sea_is_dereferenceable(cond, sizeof(pthread_cond_t)));
  return 0;
}

/**
 * @brief Broadcast a condition to all threads.
 *
 * The pthread_cond_broadcast() function unblocks all threads currently blocked
 * on the specified condition variable `cond`. It's typically used to signal
 * that a particular state has changed which multiple threads might be waiting
 * on.
 *
 * @param cond Pointer to the condition variable. It must be initialized
 *             by pthread_cond_init().
 *
 * @return On success, this function returns 0. On error, it returns a nonzero
 *         error number (such as EINVAL or EPERM).
 *
 * @note The pthread_cond_broadcast() function does not require that any
 *       threads be blocked waiting on the condition variable. If there are no
 *       threads waiting, the function does nothing and returns success.
 *
 * @warning The behavior is undefined if the value specified by the `cond`
 *          argument to pthread_cond_broadcast() does not refer to an
 *          initialized condition variable.
 */
int pthread_cond_broadcast(pthread_cond_t *cond) {
  if (nd_bool()) {
    return nd_error();
  }
  sassert(sea_is_dereferenceable(cond, sizeof(pthread_cond_t)));
  return 0;
}