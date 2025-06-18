#include <atomic>
#include <coroutine>
#include <exception>

struct Task {
	struct promise_type {
		std::atomic<bool> completed = false;

		std::suspend_always initial_suspend() noexcept { return {}; }

		struct final_awaiter : std::suspend_always {
			bool await_ready() noexcept { return false; }

			void await_suspend(std::coroutine_handle<promise_type> h) noexcept {
				h.promise().completed = true;
			}

			void await_resume() noexcept {}
		};

		final_awaiter final_suspend() noexcept { return {}; }

		Task get_return_object() {
			return Task{
				std::coroutine_handle<promise_type>::from_promise(*this)};
		}

		void return_void() {}
		void unhandled_exception() { std::terminate(); }
	};

	std::coroutine_handle<promise_type> coro;

	Task(std::coroutine_handle<promise_type> h) : coro(h) {}
	~Task() {
		if (coro && !coro.promise().completed.load()) {
			coro.destroy();
		}
	}

	void resume() {
		if (coro && !coro.done()) {
			coro.resume();
		}
	}
};