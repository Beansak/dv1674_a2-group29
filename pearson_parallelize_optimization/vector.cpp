/*
Author: David Holmqvist <daae19@student.bth.se>
*/

#include "vector.hpp"
#include <iostream>
#include <cmath>
#include <vector>

// SIMD
#include <immintrin.h>
#include <sleef.h>

Vector::Vector()
    : size{0}, data{nullptr}
{
}

Vector::~Vector()
{
    if (data)
    {
        delete[] data;
    }

    size = 0;
}

Vector::Vector(Vector &&other) noexcept
    : size(other.size),
      data(other.data),
      magnitude_cached(other.magnitude_cached),
      magnitude_cache(other.magnitude_cache)
{
    other.data = nullptr;
    other.size = 0;
}

// Move assignment operator
Vector &Vector::operator=(Vector &&other) noexcept
{
    if (this == &other)
        return *this;

    delete[] data; // Free current resources

    // Transfer ownership
    size = other.size;
    data = other.data;
    magnitude_cached = other.magnitude_cached;
    magnitude_cache = other.magnitude_cache;

    other.data = nullptr;
    other.size = 0;

    return *this;
}

Vector::Vector(unsigned size)
    : size{size}, data{new double[size]}
{
}

Vector::Vector(unsigned size, double *data)
    : size{size}, data{data}
{
}

Vector::Vector(const Vector &other)
    : Vector{other.size}
{
    for (auto i{0}; i < size; i++)
    {
        data[i] = other.data[i];
    }
}

unsigned Vector::get_size() const
{
    return size;
}

double *Vector::get_data()
{
    return data;
}

double Vector::operator[](unsigned i) const
{
    return data[i];
}

double &Vector::operator[](unsigned i)
{
    return data[i];
}

double Vector::mean() const
{
    double sum{0};

    for (auto i{0}; i < size; i++)
    {
        sum += data[i];
    }

    return sum / static_cast<double>(size);
}

double Vector::magnitude() const
{
    if (magnitude_cached)
    {
        return magnitude_cache;
    }
    magnitude_cached = true;
    magnitude_cache = std::sqrt(dot(*this));

    return magnitude_cache;
}

Vector Vector::operator/(double div)
{
    Vector result{this->size};

    for (auto i{0}; i < this->size; i++)
    {
        result.data[i] = this->data[i] / div;
    }

    return result;
}

Vector Vector::operator-(double sub)
{
    Vector result{this->size};

    for (auto i{0}; i < this->size; i++)
    {
        result.data[i] = this->data[i] - sub;
    }

    return result;
}

// Highly optimized SIMD dot product with multiple accumulation lanes
double Vector::dot(const Vector &rhs) const
{
    // Use 4 separate accumulation lanes to reduce dependency chains
    __m256d acc1 = _mm256_setzero_pd();
    __m256d acc2 = _mm256_setzero_pd();
    __m256d acc3 = _mm256_setzero_pd();
    __m256d acc4 = _mm256_setzero_pd();

    const int step = 16; // Process 16 doubles per iteration (4 AVX vectors)
    int i = 0;

    // Main SIMD loop with 4-way unrolling for better ILP (Instruction Level Parallelism)
    for (; i + step - 1 < size; i += step)
    {
        // Load 16 elements (4 AVX vectors) at once
        __m256d v1_this = _mm256_loadu_pd(&data[i]);
        __m256d v1_rhs = _mm256_loadu_pd(&rhs.data[i]);

        __m256d v2_this = _mm256_loadu_pd(&data[i + 4]);
        __m256d v2_rhs = _mm256_loadu_pd(&rhs.data[i + 4]);

        __m256d v3_this = _mm256_loadu_pd(&data[i + 8]);
        __m256d v3_rhs = _mm256_loadu_pd(&rhs.data[i + 8]);

        __m256d v4_this = _mm256_loadu_pd(&data[i + 12]);
        __m256d v4_rhs = _mm256_loadu_pd(&rhs.data[i + 12]);

        // Fused multiply-add operations (more efficient than separate mul + add)
        acc1 = _mm256_fmadd_pd(v1_this, v1_rhs, acc1);
        acc2 = _mm256_fmadd_pd(v2_this, v2_rhs, acc2);
        acc3 = _mm256_fmadd_pd(v3_this, v3_rhs, acc3);
        acc4 = _mm256_fmadd_pd(v4_this, v4_rhs, acc4);
    }

    // Handle remaining elements in groups of 4
    const int step_small = 4;
    for (; i + step_small - 1 < size; i += step_small)
    {
        __m256d v_this = _mm256_loadu_pd(&data[i]);
        __m256d v_rhs = _mm256_loadu_pd(&rhs.data[i]);
        acc1 = _mm256_fmadd_pd(v_this, v_rhs, acc1);
    }

    // Combine all accumulators
    acc1 = _mm256_add_pd(acc1, acc2);
    acc3 = _mm256_add_pd(acc3, acc4);
    acc1 = _mm256_add_pd(acc1, acc3);

    // Horizontal sum of the final accumulator using efficient method
    __m128d sum_high = _mm256_extractf128_pd(acc1, 1);
    __m128d sum_low = _mm256_castpd256_pd128(acc1);
    sum_low = _mm_add_pd(sum_low, sum_high);
    __m128d sum_final = _mm_hadd_pd(sum_low, sum_low);

    double result = _mm_cvtsd_f64(sum_final);

    // Handle remaining scalar elements
    for (; i < size; i++)
    {
        result += data[i] * rhs.data[i];
    }

    return result;
}