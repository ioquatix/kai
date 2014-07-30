
#include <UnitTest/UnitTest.hpp>

#include <Kai/Memory/ManagedObject.hpp>
#include <Kai/Memory/Collector.hpp>
#include <Kai/Reference.hpp>

namespace Kai
{
	namespace Memory
	{
		UnitTest::Suite MemoryTestSuite {
			"Kai::Memory",

			{"Allocation",
				[](UnitTest::Examiner & examiner) {
					Memory::PageAllocation * allocator = Memory::PageAllocation::create(128 * Memory::page_size());
					
					{
						Ref<ManagedObject> object = new(allocator) ManagedObject;
					
						examiner << "Object was allocated within the page boundaries." << std::endl;
						examiner.check(allocator->includes(object));
					
						examiner << "Object was retained." << std::endl;
						examiner.check_equal(object->reference_count(), 1);
					}
					
					examiner.check_equal(allocator->allocation_count(), 4);
					
					Collector collector(allocator);
					
					examiner << "Freed one allocation." << std::endl;
					examiner.check_equal(collector.collect(), 1);
				}
			},
		};
	}
}
