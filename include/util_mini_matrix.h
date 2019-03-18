#ifndef UTIL_MINIMATRIX_H
#define UTIL_MINIMATRIX_H

#include <exception>
#include <stdexcept>
#include <iostream>
#include <complex>

/** @defgroup PackageMiniMatrix  MiniMatrix
 *  A Matrix template class family used for matrix caculations for small matrixes
 *  The original application this template class family targed is 802.11n MIMO
 *  matrix calculation.
 */

/** declare template class and their friend functions first
 *  to avoid compilation errors. See c++ faq lite 35.16
 */
template<typename T> class MiniVector;
template<typename T> class MiniMatrix;

template<typename T>
std::ostream& operator<< (std::ostream& o,
                         const MiniMatrix<T>& matrix);

template<typename T>
std::ostream& operator<< (std::ostream& o,
                         const MiniVector<T>& vector);

/** @ingroup PackageMiniMatrix
 *  A base template class representing a general vector;
 */
template<typename T>
class MiniVector
{
  public:
    MiniVector() { m_Size = 0; }
    MiniVector(std::size_t size) : m_Size(size)
    {
        if (m_Size > MAX_SIZE )
        {
            throw std::out_of_range("Vector is oversized!");
        }
        m_Size = size;
//        for (int i = 0; i < m_Size; i++) {
//            m_Pdata[i] = static_cast<T>(0);
//        }
    }

    MiniVector(const MiniVector& vector)
    {
        this->Clone(vector);
    }

    MiniVector& operator=(const MiniVector& vector)
    {
        if (this != &vector)
        {
            this->Clone(vector);
        }
        return *this;
    }

    ~MiniVector() {}

    /** operator()
     */
    T   operator[](unsigned int idx) const
    {
        if (idx >= m_Size)
        {
           throw std::out_of_range("Out of vector subscript range!");
        }
        return m_Pdata[idx];
    }

    /** operator()
     */
    T&  operator[](unsigned int idx)
    {
        if (idx >= m_Size)
        {
           throw std::out_of_range("Out of vector subscript range!");
        }
        return m_Pdata[idx];
    }

    /*
     * operator *=
     */
    MiniVector& operator*=(const T& scalar)
    {
        for (int i = 0; i < m_Size; i++)
        {
            m_Pdata[i] *= scalar;
        }
        return *this;
    }

    /*
     * * Push_back()
     */
    void Push_back(const T& element) {
        if (m_Size >= MAX_SIZE) {
            throw std::out_of_range("out of vector subscript range!");
        }
        m_Pdata[m_Size++] = element;
    }

    /*
     * Clear()
     */
    void Clear() {
        m_Size = 0;
    }

    /*
     * Size()
     */
    std::size_t Size() const
    {
        return m_Size;
    }

    /** operator<<
     * used for printing out the content of the matrix
     */
    friend std::ostream& operator<< <> (std::ostream& o,
                              const MiniVector<T>& vector);

  private:
    static std::size_t const MAX_SIZE = 4;

    std::size_t  m_Size;
    T            m_Pdata[MAX_SIZE];

    void Clone(const MiniVector& vector) {
        m_Size = vector.m_Size;
        for (int i = 0; i < m_Size; i++) {
            m_Pdata[i] = vector.m_Pdata[i];
        }
    }
};

/** @ingroup PackageMiniMatrix
 *  A base template class representing a general matrix;
 */
template <typename T>
class MiniMatrix
{
  public:
    static MiniMatrix IdentitySquareMatrix(std::size_t size);

    /** operator<<
     * used for printing out the content of the matrix
     */
    friend std::ostream& operator<< <> (std::ostream& o,
                              const MiniMatrix<T>& matrix);

    MiniMatrix()
    {
        m_Rows = m_Cols = 0;
    }
    MiniMatrix(std::size_t rows, std::size_t cols)
                  : m_Rows(rows), m_Cols(cols)
    {
        if (rows > MAX_SIZE || cols > MAX_SIZE)
        {
            throw std::out_of_range("Matrix is oversized!");
        }
//        for (int i = 0; i < m_Rows; i++)
//        {
//            for (int j = 0; j < m_Cols; j++)
//            {
//                m_Pdata[i*m_Cols + j] = static_cast<T>(0);
//            }
//        }
    }

    MiniMatrix(const MiniMatrix& matrix)
    {
        this->Clone(matrix);
    }
    MiniMatrix& operator=(const MiniMatrix& matrix)
    {
        if (this != &matrix)
        {
            this->Clone(matrix);
        }
        return *this;
    }

    ~MiniMatrix() {}

    /** operator()
     *  used to get the value of matrix[row][col]
     *  See c++ faq lite 13.10 for why () instead of [][] is used
     */
    T   operator()(unsigned int row, unsigned int col) const;

    /** operator()
     *  used to set the value of matrix[row][col]
     */
    T&  operator()(unsigned int row, unsigned int col);

    /** Clear()
     *  reset all data as zero
     */
    void Clear()
    {
//        for (int i = 0; i < m_Rows; i++)
//        {
//            for (int j = 0; j < m_Cols; j++)
//            {
//                m_Pdata[i*m_Cols + j] = static_cast<T>(0);
//            }
//        }
        m_Rows = 0;
        m_Cols = 0;
    }

    MiniMatrix operator*(const MiniMatrix& rightMatrix) const;
    MiniMatrix operator*(const T& scalar) const;
    MiniMatrix operator+(const MiniMatrix& rightMatrix) const;
    MiniMatrix operator-(const MiniMatrix& rightMatrix) const;
    MiniMatrix operator/(const T& scalar) const;
    MiniMatrix Transpose() const;
    T          Determinant() const;
    MiniMatrix Inverse() const;

    std::size_t Rows() const { return m_Rows; }
    std::size_t Cols() const { return m_Cols; }
    bool isSquare() const { return m_Rows == m_Cols; }

    /** GetSubSquareMatrix()
     *  Get the sub square matrix of the current matrix
     */
    MiniMatrix GetSubSquareMatrix(std::size_t size) const;

  private:
    static std::size_t const MAX_SIZE = 4;

    std::size_t  m_Rows;
    std::size_t  m_Cols;
    T            m_Pdata[MAX_SIZE*MAX_SIZE];

    /** Clone()
     *  called by subclasses' copy constructors or assignment operator
     *  to duplicate matrix data
     */
    void Clone(const MiniMatrix& matrix);

    /** Minor()
     *  Get the minor matrix of the current matrix
     *  for calculating determinant and cofactor matrices
     */
    MiniMatrix Minor(int row, int col) const;
    
    /** Adjoint()
     *  Get the Adjoint matrix of the current matrix,
     *  which is the transpose of cofactor matrix.
     */
    MiniMatrix Adjoint() const;

};

typedef MiniVector<std::complex<double> >  CplxMiniVector;
typedef MiniMatrix<std::complex<double> >  CplxMiniMatrix;

template<typename T>
std::ostream& operator<< (std::ostream& o, const MiniVector<T>& vector)
{
    for (int j = 0; j < vector.m_Size; j++)
    {
        o << vector.m_Pdata[j] << "   ";
    }
    o << std::endl;
}

template<typename T>
std::ostream& operator<< (std::ostream& o, const MiniMatrix<T>& matrix)
{
    for (int i = 0; i < matrix.m_Rows; i++)
    {
        for (int j = 0; j < matrix.m_Cols; j++)
        {
            o << matrix.m_Pdata[i*matrix.m_Cols + j] << "   ";
        }
        o << std::endl;
    }
    return o << std::endl;
}

template<typename T>
MiniMatrix<T> MiniMatrix<T>::operator*(const MiniMatrix<T>& right) const
{
    if (m_Cols != right.m_Rows)
    {
        throw std::out_of_range("Matrices do not match for Multiply operation!");
    }
    MiniMatrix<T> res(m_Rows, right.m_Cols);
    for (int i = 0; i < m_Rows; i++)
    {
        for (int j = 0; j < right.m_Cols; j++)
        {
            T sum = static_cast<T>(0);
            for (int k = 0; k < m_Cols; k++)
            {
                sum += m_Pdata[i*m_Cols + k]*
                    right.m_Pdata[k*right.m_Cols + j];
            }
            res.m_Pdata[i*right.m_Cols + j] = sum;
        }
    }
    return res;
}

template<typename T>
MiniMatrix<T> MiniMatrix<T>::operator*(const T& scalar) const
{
    MiniMatrix<T> res(m_Rows, m_Cols);
    for (int i = 0; i < m_Rows; i++)
    {
        for (int j = 0; j < m_Cols; j++)
        {
            res.m_Pdata[i*m_Cols + j] = m_Pdata[i*m_Cols + j]*scalar;
        }
    }
    return res;
}

template<typename T>
MiniMatrix<T> MiniMatrix<T>::operator/(const T& scalar) const
{
    MiniMatrix<T> res(m_Rows, m_Cols);
    for (int i = 0; i < m_Rows; i++)
    {
        for (int j = 0; j < m_Cols; j++)
        {
            res.m_Pdata[i*m_Cols + j] = m_Pdata[i*m_Cols + j]/scalar;
        }
    }
    return res;
}

template<typename T>
MiniMatrix<T> MiniMatrix<T>::operator+(const MiniMatrix<T>& rightMatrix) const
{
    if (m_Rows != rightMatrix.m_Rows || m_Cols != rightMatrix.m_Cols)
    {
        throw std::out_of_range("Matrices do not match for Add operation!");
    }
    MiniMatrix<T> res(m_Rows, m_Cols);
    for (int i = 0; i < m_Rows; i++)
    {
        for (int j = 0; j < m_Cols; j++)
        {
            res.m_Pdata[i*m_Cols + j] = m_Pdata[i*m_Cols + j]
                + rightMatrix.m_Pdata[i*m_Cols + j];
        }
    }
    return res;
}

template<typename T>
MiniMatrix<T> MiniMatrix<T>::operator-(const MiniMatrix<T>& rightMatrix) const
{
    if (m_Rows != rightMatrix.m_Rows || m_Cols != rightMatrix.m_Cols)
    {
        throw std::out_of_range("Matrices do not match for Subtract operation!");
    }
    MiniMatrix<T> res(m_Rows, m_Cols);
    for (int i = 0; i < m_Rows; i++)
    {
        for (int j = 0; j < m_Cols; j++)
        {
            res.m_Pdata[i*m_Cols + j] = m_Pdata[i*m_Cols + j]
                - rightMatrix.m_Pdata[i*m_Cols + j];
        }
    }
    return res;
}

template<typename T>
inline T MiniMatrix<T>::operator()
        (unsigned int row, unsigned int col) const
{
    if (row >= m_Rows || col >= m_Cols)
    {
       throw std::out_of_range("Out of matrix subscript range!");
    }
    return m_Pdata[row * m_Cols + col];
}

template<typename T>
inline T& MiniMatrix<T>::operator()
        (unsigned int row, unsigned int col)
{
    if (row >= m_Rows || col >= m_Cols)
    {
       throw std::out_of_range("Out of matrix subscript range!");
    }
    return m_Pdata[row*m_Cols + col];
}

template<typename T>
MiniMatrix<T> MiniMatrix<T>::Transpose() const
{
    MiniMatrix<T>  resMatrix(m_Cols, m_Rows);
    for (int i = 0; i < m_Rows; i++)
    {
        for (int j = 0; j < m_Cols; j++)
        {
            resMatrix.m_Pdata[j*m_Rows+i] = m_Pdata[i*m_Cols+j];
        }
    }
    return resMatrix;
}

template<typename T>
T MiniMatrix<T>::Determinant() const
{
    if (!this->isSquare())
    {
        throw std::domain_error("Not applicable operation!");
    }
    T determinant;

    if (m_Rows == 1)
    {
        determinant = m_Pdata[0];
    }
    else if (m_Rows == 2)
    {
        determinant = m_Pdata[0]*m_Pdata[3] - m_Pdata[1]*m_Pdata[2];
    }
    else if (m_Rows == 3)
    {
        determinant = m_Pdata[0]*m_Pdata[4]*m_Pdata[8]
                        - m_Pdata[0]*m_Pdata[5]*m_Pdata[7]
                        - m_Pdata[1]*m_Pdata[3]*m_Pdata[8]
                        + m_Pdata[1]*m_Pdata[5]*m_Pdata[6]
                        + m_Pdata[2]*m_Pdata[3]*m_Pdata[7]
                        - m_Pdata[2]*m_Pdata[4]*m_Pdata[6];
    }
    else
    {
        determinant = static_cast<T>(0);
        for (int j = 0; j < m_Cols; j++)
        {
            T detM = this->Minor(0, j).Determinant();
            if (j&1)        //i == 0; check whether i+j is odd
            {
                detM = -detM;
            }
            determinant += detM*m_Pdata[j];
        }
    }
    return determinant;
}

template<typename T>
MiniMatrix<T> MiniMatrix<T>::Inverse() const
{
    if (!this->isSquare())
    {
        throw std::domain_error("Not applicable operation!");
    }
    if (m_Rows == 1)
    {
        throw std::domain_error("Not applicable operation!");
    }
    T determinant = this->Determinant();
    //TODO: Test zero with relative comparison
    if (determinant == static_cast<T>(0))
    {
        throw std::overflow_error("Matrix is not inversible!");
    }

    MiniMatrix<T> inverseMatrix = (this->Adjoint())/determinant;
    return inverseMatrix;
}

template<typename T>
void MiniMatrix<T>::Clone(const MiniMatrix<T>& matrix)
{
    m_Rows = matrix.m_Rows;
    m_Cols = matrix.m_Cols;
    for (int i = 0; i < m_Rows; i++)
    {
        for (int j = 0; j < m_Cols; j++)
        {
            m_Pdata[i*m_Cols + j] = matrix.m_Pdata[i*m_Cols + j];
        }
    }
}

template<typename T>
MiniMatrix<T> MiniMatrix<T>::Minor(int row, int col) const
{
    if (!this->isSquare())
    {
        throw std::domain_error("Not applicable operation!");
    }
    MiniMatrix<T> minorMatrix(m_Rows-1, m_Cols-1);
    int i, j, k, m;
    for (i = 0, k = 0; i < m_Rows; i++)
    {
        if (i == row)
            continue;
        for (j = 0, m = 0; j < m_Cols; j++)
        {
            if (j == col)
                continue;
            minorMatrix(k,m) = this->operator()(i,j);
            m++;
        }
        k++;
    }

    return minorMatrix;
}

template<typename T>
MiniMatrix<T> MiniMatrix<T>::Adjoint() const
{
    if (!this->isSquare())
    {
        throw std::domain_error("Not applicable operation!");
    }
    MiniMatrix<T> adjoint(m_Cols, m_Rows);

    for (int i = 0; i < m_Cols; i++)
    {
        for (int j = 0; j < m_Rows; j++)
        {
            adjoint(j, i) = this->Minor(i, j).Determinant();
            //if (i+j) is odd, then negative
            if ((i+j)&1)
            {
                adjoint(j, i) = -adjoint(j,i);
            }
        }
    }
    return adjoint;
}

/*
 * VectorMultiply
 * Multiple two vectors to get a matrix
 */
template<typename T>
MiniMatrix<T> VectorMultiply(const MiniVector<T>& leftVector,
                             const MiniVector<T>& rightVector)
{
    MiniMatrix<T> resMatrix(leftVector.Size(), rightVector.Size());
    for (int i = 0; i < leftVector.Size(); i++) {
        for (int j = 0; j < rightVector.Size(); j++) {
            resMatrix(i, j) = leftVector[i]*rightVector[j];
        }
    }
    return resMatrix;
}

template<typename T>
MiniMatrix<T> MiniMatrix<T>::IdentitySquareMatrix(std::size_t size) 
{
    if (size > MAX_SIZE)
    {
        throw std::out_of_range("size exceeds the maximum limit!");
    }

    MiniMatrix<T> resMatrix(size, size);

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (i == j) {
                resMatrix.m_Pdata[j*size+i] = static_cast<T>(1);
            }
            else {
                resMatrix.m_Pdata[j*size+i] = static_cast<T>(0);
            }
        }
    }

    return resMatrix;
}

template<typename T>
MiniMatrix<T> MiniMatrix<T>::GetSubSquareMatrix(std::size_t size) const
{
    MiniMatrix<T> subMt(size, size);
    if (size > m_Rows || size > m_Cols)
    {
        throw std::out_of_range("out of range!");
    }

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
           subMt(i,j) = m_Pdata[i*m_Cols+j];
        }
    }
    return subMt;
}

#endif //UTIL_MINIMATRIX_H
