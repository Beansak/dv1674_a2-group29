#pragma once

class Vector
{
private:
    unsigned size;
    double *data;
    mutable bool magnitude_cached{false};
    mutable double magnitude_cache{0.0};

public:
    Vector();
    Vector(unsigned size);
    Vector(unsigned size, double *data);
    Vector(const Vector &other);     // Copy constructor
    Vector(Vector &&other) noexcept; // Move constructor (ADD THIS)
    ~Vector();

    double magnitude() const;
    double mean() const;
    double normalize() const;
    double dot(const Vector &rhs) const;

    unsigned get_size() const;
    double *get_data();

    Vector operator/(double div);
    Vector operator-(double sub);
    double operator[](unsigned i) const;
    double &operator[](unsigned i);

    Vector &operator=(const Vector &other);     // Copy assignment
    Vector &operator=(Vector &&other) noexcept; // Move assignment (ADD THIS)
};