#pragma once

#include <vector>
#include <iostream>
#include <stdexcept>
#include <string>

constexpr int MAX_MAT_SIZE = 5;
constexpr int MAX_ALLOWED_VALUE = 1000;
constexpr int MIN_ALLOWED_VALUE = -1024;

template <typename T>
class SquareMatrix
{
public:
    SquareMatrix(const SquareMatrix&) = default;
    SquareMatrix(SquareMatrix&&) = default;
    SquareMatrix& operator=(const SquareMatrix&) = default;
    SquareMatrix& operator=(SquareMatrix&&) = default;
    ~SquareMatrix() = default;

    SquareMatrix(int size, const T& value);
    SquareMatrix(int size);

    int size() const { return m_size; }
    T& operator()(int i, int j);
    const T& operator()(int i, int j) const;

    SquareMatrix& operator+=(const SquareMatrix& rhs);
    SquareMatrix& operator-=(const SquareMatrix& rhs);
    SquareMatrix operator+(const SquareMatrix& rhs) const;
    SquareMatrix operator-(const SquareMatrix& rhs) const;
    SquareMatrix operator*(const T& scalar) const;
    SquareMatrix Transpose() const;

private:
    int m_size;
    std::vector<std::vector<T>> m_matrix;

    void validateMatrixRange() const;
};

template <typename T>
const T& SquareMatrix<T>::operator()(int i, int j) const
{
    return m_matrix[i][j];
}

template <typename T>
T& SquareMatrix<T>::operator()(int i, int j)
{
    return m_matrix[i][j];
}

inline std::ostream& operator<<(std::ostream& ostr, const SquareMatrix<int>& matrix)
{
    for (int i = 0; i < matrix.size(); ++i)
    {
        for (int j = 0; j < matrix.size(); ++j)
        {
            ostr << matrix(i, j) << ' ';
        }
        ostr << '\n';
    }
    return ostr;
}

inline std::istream& operator>>(std::istream& istr, SquareMatrix<int>& matrix)
{
    for (int i = 0; i < matrix.size(); ++i)
    {
        for (int j = 0; j < matrix.size(); ++j)
        {
            int value;
            istr >> value;

            if (!istr)
                throw std::invalid_argument("Expected numeric matrix element.");

            if (value < MIN_ALLOWED_VALUE || value > MAX_ALLOWED_VALUE)
                throw std::invalid_argument(
                    "Matrix element out of allowed range [" +
                    std::to_string(MIN_ALLOWED_VALUE) + ", " +
                    std::to_string(MAX_ALLOWED_VALUE) + "]");

            matrix(i, j) = value;
        }
    }
    return istr;
}

template <typename T>
SquareMatrix<T>::SquareMatrix(int size, const T& value)
    : m_size(size), m_matrix(size, std::vector<T>(size, value)) {
}

template <typename T>
SquareMatrix<T>::SquareMatrix(int size)
    : m_size(size), m_matrix(size, std::vector<T>(size)) {
}

template <typename T>
SquareMatrix<T> SquareMatrix<T>::operator+(const SquareMatrix& rhs) const
{
    SquareMatrix result(*this);
    result += rhs;
    return result;
}

template <typename T>
SquareMatrix<T> SquareMatrix<T>::operator-(const SquareMatrix& rhs) const
{
    SquareMatrix result(*this);
    result -= rhs;
    return result;
}

template <typename T>
SquareMatrix<T>& SquareMatrix<T>::operator+=(const SquareMatrix& rhs)
{
    for (int i = 0; i < m_size; ++i)
    {
        for (int j = 0; j < m_size; ++j)
        {
            m_matrix[i][j] += rhs.m_matrix[i][j];
        }
    }
    validateMatrixRange();  //בדיקת חריגה
    return *this;
}

template <typename T>
SquareMatrix<T>& SquareMatrix<T>::operator-=(const SquareMatrix& rhs)
{
    for (int i = 0; i < m_size; ++i)
    {
        for (int j = 0; j < m_size; ++j)
        {
            m_matrix[i][j] -= rhs.m_matrix[i][j];
        }
    }
    validateMatrixRange();  //בדיקת חריגה
    return *this;
}

template <typename T>
SquareMatrix<T> SquareMatrix<T>::operator*(const T& scalar) const
{
    SquareMatrix result(*this);
    for (int i = 0; i < m_size; ++i)
    {
        for (int j = 0; j < m_size; ++j)
        {
            result(i, j) *= scalar;
        }
    }
    result.validateMatrixRange();  //בדיקת חריגה
    return result;
}

template <typename T>
SquareMatrix<T> SquareMatrix<T>::Transpose() const
{
    SquareMatrix result(m_size);
    for (int i = 0; i < m_size; ++i)
    {
        for (int j = 0; j < m_size; ++j)
        {
            result(i, j) = m_matrix[j][i];
        }
    }
    return result;
}

template <typename T>
void SquareMatrix<T>::validateMatrixRange() const
{
    for (int i = 0; i < m_size; ++i)
    {
        for (int j = 0; j < m_size; ++j)
        {
            const T& val = m_matrix[i][j];
            if (val < MIN_ALLOWED_VALUE || val > MAX_ALLOWED_VALUE)
                throw std::invalid_argument(
                    "Computed matrix value out of range [" +
                    std::to_string(MIN_ALLOWED_VALUE) + ", " +
                    std::to_string(MAX_ALLOWED_VALUE) + "]");
        }
    }
}
