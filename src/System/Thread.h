#pragma once

#include <Core/Core.h>
#include <thread>
#include <atomic>
#include <stop_token>

namespace Vortex {

/// A thread that performs a task, running in the background.
class BackgroundThread {
   public:
    virtual ~BackgroundThread();

    BackgroundThread();

    /// Creates a thread, which calls "exec" once, and then terminates. The
    /// function returns when the thread is created; use "waitUntilDone" to wait
    /// until the thread has terminated.
    void start();

    /// Sets the terminate flag and waits until the thread is terminated. The
    /// terminate flag is only a request; The "exec" function is responsible for
    /// testing the flag and returning.
    void terminate();

    /// Waits until the thread has terminated, after which the function returns.
    void waitUntilDone();

    /// Returns a token that "exec" can poll to find out whether termination has
    /// been requested. The token is valid from construction onwards, so it is
    /// safe to take one before, during or after "start".
    std::stop_token getStopToken() const;

    /// Returns true if the thread has terminated, false if the thread is still
    /// running.
    bool isDone() const;

    /// The worker function called by the thread created in "start".
    virtual void exec() = 0;

   private:
    // The cancellation flag deliberately lives in a stop_source of our own
    // rather than in the jthread's built-in one. A default-constructed jthread
    // has no stop state at all, and "start" replaces the whole jthread object,
    // so a token taken from it before "start" is permanently dead and a token
    // taken from inside the worker races with the assignment in "start".
    // Declared before "thread" so the worker is joined while this is still
    // alive.
    std::stop_source stopSource;
    std::jthread thread;
    std::atomic_bool done;
};

};  // namespace Vortex
