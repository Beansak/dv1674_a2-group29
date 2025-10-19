/*
Author: David Holmqvist <daae19@student.bth.se>
*/

#include "vector.hpp"
#include <iostream>
#include <cmath>
#include <vector>

//SIMD
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
    auto dot_prod{dot(*this)};
    return std::sqrt(dot_prod);
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

// [I] SIMD could be used here for optimization
double Vector::dot(const Vector& rhs) const
{
    double result{0};
    double partial[4]; // temporary array to hold partial results
    int step = 4;

    int i = 0;
    for (; i + step - 1 < size; i += step)
    {
        //SIMD version using AVX intrinsics
        __m256d v_this = _mm256_loadu_pd(&data[i]); // load 4 doubles from this->data
        __m256d v_rhs = _mm256_loadu_pd(&rhs.data[i]);
        __m256d v_mul = _mm256_mul_pd(v_this, v_rhs); // multiply element-wise
        
        // horizontal add to get the sum of the 4 products
        
        _mm256_storeu_pd(partial, v_mul); // Store the 4 doubles into an array
        result += partial[0] + partial[1] + partial[2] + partial[3];
    }

    for (; i < size; i++)
    {
        result += data[i] * rhs[i];
    }

    return result;
}