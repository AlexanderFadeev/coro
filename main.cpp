#include <iostream>
#include <assert.h>

using namespace std;

class Coro {
public:;
	virtual void update() = 0;
	virtual bool done() {
		return _done;
	}

	virtual ~Coro() {
		assert(_done);
	}

protected:
	Coro() = default;
	uint8_t state = 0;
	bool _done = false;
};

#define CoroBegin()        \
	if (_done) return;      \
	switch (this->state) { \
		case 0:

#define CoroEnd() \
	}			  \
	_done = true;

#define CoroYield()                          \
	state = __LINE__; return; case __LINE__:

class CoroA : public Coro {
public:
	void update() override {
		CoroBegin()

		cout << "Hello\n";
		CoroYield();
		cout << "World\n";
		CoroYield();
		cout << "Hello\n";
		CoroYield();
		cout << "World\n";

		CoroEnd()
	}
};

class CoroB : public Coro {
public:
	void update() override {
		CoroBegin()

		cout << "Foo\n";
		CoroYield();
		cout << "Bar\n";
		CoroYield();
		cout << "Baz\n";

		CoroEnd()
	}
};

class CoroC : public Coro {
private:
	int i = 0;
	int sum = 0;
public:
	void update() override {
		CoroBegin()

		for (i = 0; i <= 10; i++) {
			sum += i;
			cout << sum << endl;
			CoroYield();
		}

		CoroEnd()
	}
};

template <typename ...Args>
class MultiCoro : public Coro {
private:
	inline void init(int) {}

	template<typename ...Rest>
	inline void init(int idx, Coro* cur, Rest... rest) {
		coros[idx] = cur;
		init(++idx, rest...);
	}

	static constexpr size_t size = sizeof...(Args);
	Coro* coros[size];

public:
	MultiCoro(Args... args) {
		init(0, args...);
	}

	void update() override {
		if (_done) {
			return;
		}

		_done = true;
		for (size_t idx = 0; idx < size; idx++) {
			coros[idx]->update();
			_done &= coros[idx]->done();
		}
	}
};

int main() {
	CoroA a;
	CoroB b;
	CoroC c;

	MultiCoro multi(&a, &b, &c);

	while (!multi.done()) {
		multi.update();
	}
}