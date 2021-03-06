#include <signal.h>
#include <adbase/Net.hpp>
#include <adbase/Logging.hpp>
#include <adbase/Metrics.hpp>

adbase::metrics::Metrics* metrics = nullptr;
adbase::EventLoop* gloop = nullptr;

void test(void*) {
	if (metrics != nullptr) {
		std::unordered_map<std::string, int64_t> values = metrics->getGauges();
		for (auto &t : values) {
			LOG_INFO << t.first << " -> " << t.second;
		}
	}
}

// {{{ static void killSignal()

static void killSignal(const int sig) {
	(void)sig;
	if (gloop != nullptr) {
		gloop->stop();
		delete gloop;
		gloop = nullptr;
	}

	adbase::metrics::Metrics::stop();
	exit(0);
}

// }}}
// {{{ static void reloadConf()

static void reloadConf(const int sig) {
	(void)sig;
}

// }}}
// {{{ static void registerSignal()

static void registerSignal() {
	/* 忽略Broken Pipe信号 */
	signal(SIGPIPE, SIG_IGN);
	/* 处理kill信号 */
	signal(SIGINT,  killSignal);
	signal(SIGKILL, killSignal);
	signal(SIGQUIT, killSignal);
	signal(SIGTERM, killSignal);
	signal(SIGHUP,  killSignal);
	signal(SIGSEGV, killSignal);
	signal(SIGUSR1, reloadConf);
}

// }}}

int main(void) {
	registerSignal();
	adbase::EventLoop* eventloop = new adbase::EventLoop();
	gloop = eventloop;
	adbase::Timer timer(eventloop->getBase());
	metrics = adbase::metrics::Metrics::init(&timer);

	uint32_t interval = 1000;
	timer.runEvery(interval, std::bind(test, std::placeholders::_1), nullptr);
	adbase::metrics::Metrics::buildGauges("test", "rss", interval, [](){
		return 100;
	});
	eventloop->start();
	return 0;
}
