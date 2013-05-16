
#include <Kai/Frame.h>

namespace Kai {
	class Terminal {
	public:
		static Ref<Object> import(Frame * frame) {
			std::cerr << "Loading terminal code" << std::endl;

			return NULL;
		}
	};
}

Kai::Ref<Kai::Frame> kai_load(Kai::Frame * context) {
	return Kai::Terminal::import(context);
}
