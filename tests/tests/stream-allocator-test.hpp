#ifndef FIFO_CONTIGUOUS_DATA_ALLOCATOR_TEST_HPP_INCLUDED
#define FIFO_CONTIGUOUS_DATA_ALLOCATOR_TEST_HPP_INCLUDED

#include "memory/stream-allocator.hpp"
#include "memory/trillek-allocator.hpp"

#include "gtest/gtest.h"


class StreamAllocatorTest: public ::testing::Test {
public:
    // the buffer can contain 32 double integers
    template<class T> using UnalignedAllocator = trillek::TrillekAllocator<T,trillek::memory::StreamAllocator<256>>;

    StreamAllocatorTest() : unaligned_buffer(std::make_shared<trillek::memory::StreamAllocator<256>>()) {}
protected:
    template<class Alloc>
    void FillRandom(std::vector<double,Alloc>& v, const size_t size) {
        for (size_t i = 0; i < size; i++) {
            auto t = next()/10000;
            v.push_back(t);
            witness.push_back(t);
        }
    }

    double next() {
        return ((double) (random() - (random.max()>> 1)));
    }

    std::vector<double> witness;
    unsigned long int seed = (unsigned long int) std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine random{seed};
    std::shared_ptr<trillek::memory::StreamAllocator<256>> unaligned_buffer;
};

namespace trillek {

TEST_F(StreamAllocatorTest, TripleVector) {
    UnalignedAllocator<double> alloc(unaligned_buffer.get());
    auto index = 10;
    auto myvector1 = make_unique<std::vector<double,UnalignedAllocator<double>>>(alloc);
    // we resize for 10 elements
    myvector1->reserve(index);
    FillRandom(*myvector1, index);

    auto myvector2 = make_unique<std::vector<double,UnalignedAllocator<double>>>(alloc);
    // we resize for 10 elements
    myvector2->reserve(index);
    FillRandom(*myvector2, index);

    auto index3 = 5;
    auto myvector3 = make_unique<std::vector<double,UnalignedAllocator<double>>>(alloc);
    // we resize for 6 elements
    myvector3->reserve(index3);
    FillRandom(*myvector3, index3);

    for (unsigned int i = 0; i < index + index + index3 ; i++) {
        EXPECT_EQ(witness[i], *reinterpret_cast<double*>((unaligned_buffer->data()).first + i*sizeof(double))) << "Element " << i << " (" << (void*)((unaligned_buffer->data()).first + i*sizeof(double)) << ") of buffer is not equal";
    }
    myvector1.reset();
    unaligned_buffer->gc();
    for (unsigned int i = 0; i < index + index3 ; i++) {
        EXPECT_EQ(witness[i + index], *reinterpret_cast<double*>((unaligned_buffer->data()).first + i*sizeof(double))) << "Element " << i << " (" << (void*)((unaligned_buffer->data()).first + i*sizeof(double)) << ") of buffer is not equal";
    }
    auto myvector4 = make_unique<std::vector<double,UnalignedAllocator<double>>>(alloc);
    auto index4 = 16;
    myvector4->reserve(index4);
    FillRandom(*myvector4, index4);
    for (unsigned int i = 0; i < index + index3 + index4 ; i++) {
        EXPECT_EQ(witness[i + index], *reinterpret_cast<double*>((unaligned_buffer->data()).first + i*sizeof(double))) << "Element " << i << " (" << (void*)((unaligned_buffer->data()).first + i*sizeof(double)) << ") of buffer is not equal";
    }
    myvector2.reset();
    unaligned_buffer->gc();
    for (unsigned int i = 0; i < index3 + index4 ; i++) {
        EXPECT_EQ(witness[i + 2 * index], *reinterpret_cast<double*>((unaligned_buffer->data()).first + i*sizeof(double))) << "Element " << i << " (" << (void*)((unaligned_buffer->data()).first + i*sizeof(double)) << ") of buffer is not equal";
    }
    auto myvector5 = make_unique<std::vector<double,UnalignedAllocator<double>>>(alloc);
    auto index5 = 5;
    myvector5->reserve(index5);
    FillRandom(*myvector5, index5);
    for (unsigned int i = 0; i < index3 + index4 +index5 ; i++) {
        EXPECT_EQ(witness[i + 2 * index], *reinterpret_cast<double*>((unaligned_buffer->data()).first + i*sizeof(double))) << "Element " << i << " (" << (void*)((unaligned_buffer->data()).first + i*sizeof(double)) << ") of buffer is not equal";
    }
    myvector3.reset();
    unaligned_buffer->gc();
    auto myvector6 = make_unique<std::vector<double,UnalignedAllocator<double>>>(alloc);
    auto index6 = 10;
    myvector6->reserve(index6);
    FillRandom(*myvector6, index6);
    for (unsigned int i = 0; i < index4 +index5 + index6; i++) {
        EXPECT_EQ(witness[i + 2 * index + index3], *reinterpret_cast<double*>((unaligned_buffer->data()).first + i*sizeof(double))) << "Element " << i << " (" << (void*)((unaligned_buffer->data()).first + i*sizeof(double)) << ") of buffer is not equal";
    }
    myvector4.reset();
    myvector5.reset();
    myvector6.reset();
    unaligned_buffer->gc();
    auto myvector7 = make_unique<std::vector<double,UnalignedAllocator<double>>>(alloc);
    auto index7 = 20;
    myvector7->reserve(index7);
    FillRandom(*myvector7, index7);
    for (unsigned int i = 0; i < index7 ; i++) {
        EXPECT_EQ(witness[i + 2 * index + index3 + index4 + index5 + index6], *reinterpret_cast<double*>((unaligned_buffer->data()).first + i*sizeof(double))) << "Element " << i << " (" << (void*)((unaligned_buffer->data()).first + i*sizeof(double)) << ") of buffer is not equal";
    }
}

TEST_F(StreamAllocatorTest, VectorTooBig) {
    UnalignedAllocator<double> alloc(unaligned_buffer.get());
    auto index = 40;
    std::vector<double,UnalignedAllocator<double>> myvector(alloc);
    EXPECT_THROW(myvector.reserve(index), std::bad_alloc) << "allocation of memory should throw";
}

TEST_F(StreamAllocatorTest, CopyAssigment) {
    UnalignedAllocator<double> alloc(unaligned_buffer.get());
    auto index = 6;
    auto elt = make_unique<std::vector<double,UnalignedAllocator<double>>>(alloc);
    elt->reserve(index);
    FillRandom(*elt, index);
    auto copy = make_unique<std::vector<double,UnalignedAllocator<double>>>(alloc);
    *copy = *elt;
    for (auto i = 0; i < index; ++i) {
        EXPECT_EQ(witness[i % index], *reinterpret_cast<double*>((unaligned_buffer->data()).first + i*sizeof(double))) << "Element " << i << " (" << (void*)((unaligned_buffer->data()).first + i*sizeof(double)) << ") of buffer is not equal";
    }
}

TEST_F(StreamAllocatorTest, CopyConstructor) {
    UnalignedAllocator<double> alloc(unaligned_buffer.get());
    auto index = 6;
    auto elt = make_unique<std::vector<double,UnalignedAllocator<double>>>(alloc);
    elt->reserve(index);
    FillRandom(*elt, index);
    auto copy = make_unique<std::vector<double,UnalignedAllocator<double>>>(*elt);
    for (auto i = 0; i < index; ++i) {
        EXPECT_EQ(witness[i % index], *reinterpret_cast<double*>((unaligned_buffer->data()).first + i*sizeof(double))) << "Element " << i << " (" << (void*)((unaligned_buffer->data()).first + i*sizeof(double)) << ") of buffer is not equal";
    }
}

}

#endif // FIFO_CONTIGUOUS_DATA_ALLOCATOR_TEST_HPP_INCLUDED
