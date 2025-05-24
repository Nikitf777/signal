#include <coroutine>
#include <optional>
#include <tuple>
#include <vector>

// #include "../delegate/delegate.hpp"
#include "delegate.hpp"

template <typename Ret, typename... Args> class Signal;

template <typename Ret, typename... Args> class Signal<Ret(Args...)> {
  public:
	using Signature = Ret(Args...);

	template <typename F> void Connect(F &&f) {
		m_delegate += std::forward<F>(f);
	}

	template <typename... CallArgs> void Emit(CallArgs &&...args) {
		m_delegate(std::forward<CallArgs>(args)...);

		if constexpr (sizeof...(Args)) {
			m_emissionArgs = std::make_tuple(std::forward<CallArgs>(args)...);
		}

		// Only resume active coroutines
		std::vector<std::coroutine_handle<>> toResume;
		toResume.reserve(m_awaitingCoroutines.size());

		for (auto handle : m_awaitingCoroutines) {
			if (!handle.address())
				continue; // skip null handles
			if (handle.done())
				continue; // skip completed coroutines

			toResume.push_back(handle);
		}

		// Resume all valid handles
		for (auto handle : toResume) {
			handle.resume();
		}

		// Clear list after resuming
		m_awaitingCoroutines.clear();
	}

	template <typename... CallArgs> void operator()(CallArgs &&...args) {
		Emit(std::forward<CallArgs>(args)...);
	}

	// Awaiter
	struct [[nodiscard]] Awaiter {
		Signal *signal;

		bool await_ready() const noexcept { return false; }

		void await_suspend(std::coroutine_handle<> handle) noexcept {
			signal->m_awaitingCoroutines.push_back(handle);
		}

		auto await_resume() const {
			if constexpr (sizeof...(Args) == 0) {
				return;
			} else {
				return signal->m_emissionArgs.value();
			}
		}
	};

	Awaiter operator co_await() noexcept { return Awaiter{this}; }

  private:
	Delegate<Signature> m_delegate;
	std::optional<std::tuple<Args...>> m_emissionArgs;
	std::vector<std::coroutine_handle<>> m_awaitingCoroutines;
};