#ifdef UNITTEST

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

int testmain(int argc, char* argv[]){

	trompeloeil::set_reporter([](
		trompeloeil::severity s,
		const char* file,
		unsigned long line,
		std::string const& msg) {
			std::ostringstream os;
			if (line) os << file << ':' << line << '\n';
			os << msg;
			auto failure = os.str();
			if (s == trompeloeil::severity::fatal) {
				FAIL(failure);
			}
			else {
				CAPTURE(failure);
				CHECK(failure.empty());
			}
		});
	
	return Catch::Session().run(argc, argv);
}
#endif
