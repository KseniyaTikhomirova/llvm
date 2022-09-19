//==- BufferAllocatorCheck.cpp --- check isDefault() property of Allocator--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#define SYCL2020_DISABLE_DEPRECATION_WARNINGS

#include <helpers/PiMock.hpp>
#include <helpers/TestKernel.hpp>

#include <sycl/accessor.hpp>
#include <sycl/sycl.hpp>

#include <gtest/gtest.h>

#include <detail/buffer_impl.hpp>

class BufferAllocatorCheck : public ::testing::Test {
public:
  BufferAllocatorCheck() : Mock{}, Plt{Mock.getPlatform()} {}

protected:
  void SetUp() override {
    // Mock.redefine<sycl::detail::PiApiKind::piMemBufferCreate>(
    //     redefinedMemBufferCreate);
    // Mock.redefine<sycl::detail::PiApiKind::piDeviceGetInfo>(
    //     redefinedDeviceGetInfo);
  }

protected:
  sycl::unittest::PiMock Mock;
  sycl::platform Plt;
};

// Test that buffer_location was passed correctly
TEST_F(BufferAllocatorCheck, BufferWithSizeOnlyDefault) {
  sycl::context Context{Plt};
   if (Plt.is_host()) {
    std::cout << "Not run due to host-only environment\n";
    return;
  }
  sycl::queue Queue{Context, sycl::default_selector{}};

  sycl::buffer<int, 1> Buf(3);
  std::shared_ptr<sycl::detail::buffer_impl> BufImpl =
      sycl::detail::getSyclObjImpl(Buf);  
  ASSERT_NE(BufImpl, nullptr);
  EXPECT_TRUE(BufImpl->get_allocator_internal()->isDefault());
  EXPECT_TRUE(BufImpl->isNotBlockingRelease());
}

TEST_F(BufferAllocatorCheck, BufferWithSizeOnlyUSM) {
  sycl::context Context{Plt};
   if (Plt.is_host()) {
    std::cout << "Not run due to host-only environment\n";
    return;
  }
  sycl::queue Queue{Context, sycl::default_selector{}};
  using AllocatorTypeTest = sycl::usm_allocator<int, sycl::usm::alloc::shared>;
  AllocatorTypeTest allocator(Queue);
  sycl::buffer<int, 1, AllocatorTypeTest> Buf(3, allocator);
  std::shared_ptr<sycl::detail::buffer_impl> BufImpl =
      sycl::detail::getSyclObjImpl(Buf);  
  ASSERT_NE(BufImpl, nullptr);
  EXPECT_FALSE(BufImpl->get_allocator_internal()->isDefault());
  EXPECT_FALSE(BufImpl->isNotBlockingRelease());
}

// double timer() {
//     using namespace std::chrono;
//     auto tp = high_resolution_clock::now();
//     auto tp_duration = tp.time_since_epoch();
//     duration<double> sec = tp.time_since_epoch();
//     return sec.count() * 1000;
// }

// TEST_F(BufferAllocatorCheck, BufferDestructionDelayed) {
//   sycl::context Context{Plt};
//    if (Plt.is_host()) {
//     std::cout << "Not run due to host-only environment\n";
//     return;
//   }
  
//   double start, t1, t2;

//   sycl::queue Queue{Context, sycl::default_selector{}};
//   using buffer_u8_t = cl::sycl::buffer<uint8_t, 1>;
//   const size_t array_size = 1<<24;
//   {
//     buffer_u8_t BufferA((cl::sycl::range<1>(array_size)));

//     std::shared_ptr<sycl::detail::buffer_impl> BufImpl =
//         sycl::detail::getSyclObjImpl(Buf);  
//     ASSERT_NE(BufImpl, nullptr);
//     EXPECT_FALSE(BufImpl->get_allocator_internal()->isDefault());
//     EXPECT_FALSE(BufImpl->isNotBlockingRelease());

//           Q.submit([&](cl::sycl::handler &cgh) {
//               auto accA = BufferA.template get_access<cl::sycl::access::mode::write>(cgh);
//               cgh.parallel_for(cl::sycl::range<1>{array_size}, [=](cl::sycl::id<1> id)
//               {
//                   accA[id] = id % 2;
//               });
//           });
//           start = timer();
//           Q.submit([&](cl::sycl::handler &cgh) {
//               auto accA = BufferA.template get_access<cl::sycl::access::mode::write>(cgh);
//               cgh.parallel_for(cl::sycl::range<1>{array_size}, [=](cl::sycl::id<1> id)
//               {
//                   accA[id] = id % 2;
//               });
//           });

//           t1 = timer() - start; // before buffer destroy
//   }
//   t2 = timer() - start; // after buffer destroy

//   std::cout << "time before buffer destroy: " << t1 << " ms\n";
//   std::cout << "time  after buffer destroy: " << t2 << " ms\n";
// }

