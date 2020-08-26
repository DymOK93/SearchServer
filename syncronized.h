#include <memory>
#include <mutex>
#include <utility>
#include <functional>

template <class Ty>
class Synchronized {
public:
	Synchronized(Ty init = Ty())
		: value{std::move(init) } {}
	~Synchronized() = default;

	Synchronized(const Synchronized&) = delete;
	Synchronized& operator=(const Synchronized&) = delete;

	Synchronized(Synchronized&& other)
		: value{ std::move(other.value) } {}

	Synchronized& operator=(Synchronized&& other) {
		if (std::addressof(other) != this) {
			this->GetAccess().ref = std::move(other.value);
		}
		return *this;
	}
public:
	struct Proxy {
		std::lock_guard<std::mutex> guard;
		Ty& ref;
	};
	struct ConstProxy {
		std::lock_guard<std::mutex> guard;
		const Ty& ref;
	};
public:
	Proxy GetAccess() {
		return Proxy{ std::lock_guard<std::mutex>(mtx), value };
	}

	ConstProxy GetAccess() const {
		return ConstProxy{ std::lock_guard<std::mutex>(mtx), value };
	}
private:
	Ty value;
	mutable std::mutex mtx;
};