#include <bandit/bandit.h>
using namespace bandit;

go_bandit([](){

	describe("our first test", [](){

		it("should fail", [&](){
			AssertThat(5, Equals(5));
		});

 	});

});
