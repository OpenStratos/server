#include <bandit/bandit.h>
using namespace bandit;

go_bandit([](){

	describe("testing integration", [](){

		it("unit test", [&](){
			AssertThat(5, Equals(5));
		});

 	});

});
