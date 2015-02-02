#ifndef FIFOALLOCATOR_TEST_HPP_INCLUDED
#define FIFOALLOCATOR_TEST_HPP_INCLUDED

#include "memory/fifo-allocator.hpp"
#include "memory/trillek-allocator.hpp"

#include "gtest/gtest.h"


class FIFOAllocatorTest: public ::testing::Test {
public:
    // the buffer can contain 32 double integers
    template<class T> using UnalignedAllocator = trillek::TrillekAllocator<T,trillek::memory::FIFOAllocator<256,1>>;
    template<class T> using AlignedAllocator = trillek::TrillekAllocator<T,trillek::memory::FIFOAllocator<256>>;

    FIFOAllocatorTest() : unaligned_buffer(std::make_shared<trillek::memory::FIFOAllocator<256,1>>()),
                                    aligned_buffer(std::make_shared<trillek::memory::FIFOAllocator<256>>()) {}
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
    std::shared_ptr<trillek::memory::FIFOAllocator<256,1>> unaligned_buffer;
    std::shared_ptr<trillek::memory::FIFOAllocator<256>> aligned_buffer;
};

namespace trillek {

TEST_F(FIFOAllocatorTest, VectorDynamicResize10) {
    UnalignedAllocator<double> alloc(unaligned_buffer.get());
    auto index = 10;
    std::vector<double,UnalignedAllocator<double>> myvector(alloc);
    FillRandom(myvector, index);
    for (unsigned int i = 0; i < index ; i++) {
        EXPECT_EQ(witness[i], myvector.at(i)) << "Element " << i << " is not equal";
    }
}

TEST_F(FIFOAllocatorTest, TripleVector) {
    UnalignedAllocator<double> alloc(unaligned_buffer.get());
    auto index = 10;
    std::vector<double,UnalignedAllocator<double>> myvector1(alloc);
    // we resize dynamically to request space for 2, 4, 8 and 16 elements
    FillRandom(myvector1, index);

    std::vector<double,UnalignedAllocator<double>> myvector2(alloc);
    // we resize for 10 elements
    myvector2.reserve(index);
    FillRandom(myvector2, index);

    auto index3 = 5;
    std::vector<double,UnalignedAllocator<double>> myvector3(alloc);
    // we resize for 6 elements
    myvector3.reserve(index3);
    FillRandom(myvector3, index3);

    for (unsigned int i = 0; i < index ; i++) {
        EXPECT_EQ(witness[i], myvector1.at(i)) << "Element " << i << " of vector 1 is not equal";
        EXPECT_EQ(witness[i + index], myvector2.at(i)) << "Element " << i << " of vector 2 is not equal";
    }
    for (unsigned int i = 0; i < index3 ; i++) {
        EXPECT_EQ(witness[i + 2 * index], myvector3.at(i)) << "Element " << i << " of vector 3 is not equal";
    }
    myvector1.clear();
    myvector1.shrink_to_fit();
    std::vector<double,UnalignedAllocator<double>> myvector4(alloc);
    auto index4 = 16;
    myvector4.reserve(index4);
    FillRandom(myvector4, index4);
    for (unsigned int i = 0; i < index ; i++) {
        EXPECT_EQ(witness[i + index], myvector2.at(i)) << "Element " << i << " of vector 2 is not equal";
    }
    for (unsigned int i = 0; i < index3 ; i++) {
        EXPECT_EQ(witness[i + 2 * index], myvector3.at(i)) << "Element " << i << " of vector 3 is not equal";
    }
    for (unsigned int i = 0; i < index4 ; i++) {
        EXPECT_EQ(witness[i + 2 * index + index3], myvector4.at(i)) << "Element " << i << " of vector 1 is not equal";
    }
    myvector2.clear();
    myvector2.shrink_to_fit();
    std::vector<double,UnalignedAllocator<double>> myvector5(alloc);
    auto index5 = 5;
    myvector5.reserve(index5);
    FillRandom(myvector5, index5);
    for (unsigned int i = 0; i < index3 ; i++) {
        EXPECT_EQ(witness[i + 2 * index], myvector3.at(i)) << "Element " << i << " of vector 3 is not equal";
    }
    for (unsigned int i = 0; i < index4 ; i++) {
        EXPECT_EQ(witness[i + 2 * index + index3], myvector4.at(i)) << "Element " << i << " of vector 4 is not equal";
    }
    for (unsigned int i = 0; i < index5 ; i++) {
        EXPECT_EQ(witness[i + 2 * index + index3 + index4], myvector5.at(i)) << "Element " << i << " of vector 5 is not equal";
    }
    myvector3.clear();
    myvector3.shrink_to_fit();
    std::vector<double,UnalignedAllocator<double>> myvector6(alloc);
    auto index6 = 10;
    myvector6.reserve(index6);
    FillRandom(myvector6, index6);
    for (unsigned int i = 0; i < index4 ; i++) {
        EXPECT_EQ(witness[i + 2 * index + index3], myvector4.at(i)) << "Element " << i << " of vector 4 is not equal";
    }
    for (unsigned int i = 0; i < index5 ; i++) {
        EXPECT_EQ(witness[i + 2 * index + index3 + index4], myvector5.at(i)) << "Element " << i << " of vector 5 is not equal";
    }
    for (unsigned int i = 0; i < index6 ; i++) {
        EXPECT_EQ(witness[i + 2 * index + index3 + index4 + index5], myvector6.at(i)) << "Element " << i << " of vector 6 is not equal";
    }
    myvector4.clear();
    myvector4.shrink_to_fit();
    myvector5.clear();
    myvector5.shrink_to_fit();
    myvector6.clear();
    myvector6.shrink_to_fit();
    std::vector<double,UnalignedAllocator<double>> myvector7(alloc);
    auto index7 = 20;
    myvector7.reserve(index7);
    FillRandom(myvector7, index7);
    for (unsigned int i = 0; i < index7 ; i++) {
        EXPECT_EQ(witness[i + 2 * index + index3 + index4 + index5 + index6], myvector7.at(i)) << "Element " << i << " of vector 7 is not equal";
    }
}

TEST_F(FIFOAllocatorTest, VectorTooBig) {
    UnalignedAllocator<double> alloc(unaligned_buffer.get());
    auto index = 40;
    std::vector<double,UnalignedAllocator<double>> myvector(alloc);
    EXPECT_THROW(myvector.reserve(index), std::bad_alloc) << "allocation of memory should throw";
}

TEST_F(FIFOAllocatorTest, MultipleBuffers) {
    UnalignedAllocator<double> alloc(unaligned_buffer.get());
    std::vector<std::vector<double,UnalignedAllocator<double>>> v(50, std::vector<double,UnalignedAllocator<double>>(alloc));
    auto index = 20;
    for (auto& elt : v) {
        elt.reserve(index);
        FillRandom(elt, index);
    }
    auto i = 0;
    for(auto& elt : v) {
        for (auto& c : elt) {
            EXPECT_EQ(c,witness[i++]);
        }
    }
}

TEST_F(FIFOAllocatorTest, CopyAssigment) {
    UnalignedAllocator<double> alloc(unaligned_buffer.get());
    std::vector<std::vector<double,UnalignedAllocator<double>>> v(50, std::vector<double,UnalignedAllocator<double>>(alloc));
    auto index = 12;
    for (auto& elt : v) {
        elt.reserve(index);
        FillRandom(elt, index);
    }
    std::vector<std::vector<double,UnalignedAllocator<double>>> copy{};
    copy = v;
    auto i = 0;
    for(auto& elt : copy) {
        for (auto& c : elt) {
            EXPECT_EQ(c,witness[i++]);
        }
    }
}

TEST_F(FIFOAllocatorTest, MoveAssignment) {
    UnalignedAllocator<double> alloc(unaligned_buffer.get());
    std::vector<std::vector<double,UnalignedAllocator<double>>> v(50, std::vector<double,UnalignedAllocator<double>>(alloc));
    auto index = 12;
    for (auto& elt : v) {
        elt.reserve(index);
        FillRandom(elt, index);
    }
    std::vector<std::vector<double,UnalignedAllocator<double>>> copy{};
    copy = std::move(v);
    auto i = 0;
    for(auto& elt : copy) {
        for (auto& c : elt) {
            EXPECT_EQ(c,witness[i++]);
        }
    }
}

TEST_F(FIFOAllocatorTest, CopyConstructor) {
    UnalignedAllocator<double> alloc(unaligned_buffer.get());
    std::vector<std::vector<double,UnalignedAllocator<double>>> v(50, std::vector<double,UnalignedAllocator<double>>(alloc));
    auto index = 12;
    for (auto& elt : v) {
        elt.reserve(index);
        FillRandom(elt, index);
    }
    auto copy = v;
    auto i = 0;
    for(auto& elt : copy) {
        for (auto& c : elt) {
            EXPECT_EQ(c,witness[i++]);
        }
    }
}

TEST_F(FIFOAllocatorTest, MoveConstructor) {
    UnalignedAllocator<double> alloc(unaligned_buffer.get());
    std::vector<std::vector<double,UnalignedAllocator<double>>> v(50, std::vector<double,UnalignedAllocator<double>>(alloc));
    auto index = 12;
    for (auto& elt : v) {
        elt.reserve(index);
        FillRandom(elt, index);
    }
    auto copy = std::move(v);
    auto i = 0;
    for(auto& elt : copy) {
        for (auto& c : elt) {
            EXPECT_EQ(c,witness[i++]);
        }
    }
}

TEST_F(FIFOAllocatorTest, AlignedVectorDynamicResize10) {
    AlignedAllocator<double> alloc(aligned_buffer.get());
    auto index = 10;
    std::vector<double,AlignedAllocator<double>> myvector(alloc);
    FillRandom(myvector, index);
    for (unsigned int i = 0; i < index ; i++) {
        EXPECT_EQ(witness[i], myvector.at(i)) << "Element " << i << " is not equal";
    }
}
TEST_F(FIFOAllocatorTest, AlignedTripleVector) {
    AlignedAllocator<double> alloc(aligned_buffer.get());
    auto index = 10;
    std::vector<double,AlignedAllocator<double>> myvector1(alloc);
    // we resize dynamically to request space for 2, 4, 8 and 16 elements
    FillRandom(myvector1, index);

    std::vector<double,AlignedAllocator<double>> myvector2(alloc);
    // we resize for 10 elements
    myvector2.reserve(index);
    FillRandom(myvector2, index);

    auto index3 = 5;
    std::vector<double,AlignedAllocator<double>> myvector3(alloc);
    // we resize for 6 elements
    myvector3.reserve(index3);
    FillRandom(myvector3, index3);

    for (unsigned int i = 0; i < index ; i++) {
        EXPECT_EQ(witness[i], myvector1.at(i)) << "Element " << i << " of vector 1 is not equal";
        EXPECT_EQ(witness[i + index], myvector2.at(i)) << "Element " << i << " of vector 2 is not equal";
    }
    for (unsigned int i = 0; i < index3 ; i++) {
        EXPECT_EQ(witness[i + 2 * index], myvector3.at(i)) << "Element " << i << " of vector 3 is not equal";
    }
    myvector1.clear();
    myvector1.shrink_to_fit();
    std::vector<double,AlignedAllocator<double>> myvector4(alloc);
    auto index4 = 16;
    myvector4.reserve(index4);
    FillRandom(myvector4, index4);
    for (unsigned int i = 0; i < index ; i++) {
        EXPECT_EQ(witness[i + index], myvector2.at(i)) << "Element " << i << " of vector 2 is not equal";
    }
    for (unsigned int i = 0; i < index3 ; i++) {
        EXPECT_EQ(witness[i + 2 * index], myvector3.at(i)) << "Element " << i << " of vector 3 is not equal";
    }
    for (unsigned int i = 0; i < index4 ; i++) {
        EXPECT_EQ(witness[i + 2 * index + index3], myvector4.at(i)) << "Element " << i << " of vector 1 is not equal";
    }
    myvector2.clear();
    myvector2.shrink_to_fit();
    std::vector<double,AlignedAllocator<double>> myvector5(alloc);
    auto index5 = 5;
    myvector5.reserve(index5);
    FillRandom(myvector5, index5);
    for (unsigned int i = 0; i < index3 ; i++) {
        EXPECT_EQ(witness[i + 2 * index], myvector3.at(i)) << "Element " << i << " of vector 3 is not equal";
    }
    for (unsigned int i = 0; i < index4 ; i++) {
        EXPECT_EQ(witness[i + 2 * index + index3], myvector4.at(i)) << "Element " << i << " of vector 4 is not equal";
    }
    for (unsigned int i = 0; i < index5 ; i++) {
        EXPECT_EQ(witness[i + 2 * index + index3 + index4], myvector5.at(i)) << "Element " << i << " of vector 5 is not equal";
    }
    myvector3.clear();
    myvector3.shrink_to_fit();
    std::vector<double,AlignedAllocator<double>> myvector6(alloc);
    auto index6 = 10;
    myvector6.reserve(index6);
    FillRandom(myvector6, index6);
    for (unsigned int i = 0; i < index4 ; i++) {
        EXPECT_EQ(witness[i + 2 * index + index3], myvector4.at(i)) << "Element " << i << " of vector 4 is not equal";
    }
    for (unsigned int i = 0; i < index5 ; i++) {
        EXPECT_EQ(witness[i + 2 * index + index3 + index4], myvector5.at(i)) << "Element " << i << " of vector 5 is not equal";
    }
    for (unsigned int i = 0; i < index6 ; i++) {
        EXPECT_EQ(witness[i + 2 * index + index3 + index4 + index5], myvector6.at(i)) << "Element " << i << " of vector 6 is not equal";
    }
    myvector4.clear();
    myvector4.shrink_to_fit();
    myvector5.clear();
    myvector5.shrink_to_fit();
    myvector6.clear();
    myvector6.shrink_to_fit();
    std::vector<double,AlignedAllocator<double>> myvector7(alloc);
    auto index7 = 20;
    myvector7.reserve(index7);
    FillRandom(myvector7, index7);
    for (unsigned int i = 0; i < index7 ; i++) {
        EXPECT_EQ(witness[i + 2 * index + index3 + index4 + index5 + index6], myvector7.at(i)) << "Element " << i << " of vector 7 is not equal";
    }
}
TEST_F(FIFOAllocatorTest, AlignedVectorTooBig) {
    AlignedAllocator<double> alloc(aligned_buffer.get());
    auto index = 40;
    std::vector<double,AlignedAllocator<double>> myvector(alloc);
    EXPECT_THROW(myvector.reserve(index), std::bad_alloc) << "allocation of memory should throw";
}

}

#endif // FIFOALLOCATOR_TEST_HPP_INCLUDED
