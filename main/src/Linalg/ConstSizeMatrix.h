/** ***********************************************************************************************
* @class		ConstSizeMatrix
* @brief		A matrix derived from Matrix for math operations with templated maximum size and memory allocated on local heap 
* @details		Details:
                    - a matrix of Real entries (Real/float);
                    - use SlimMatrix for tiny matrices with known size
                    - use LinkedDataMatrix to link data to a (part of a) matrix (without memory allocation)
                    - use ResizableMatrix to allow a matrix to allocate more data than currently needed (no memory allocation when matrix size changes)
*
* @author		Gerstmayr Johannes
* @date			1997-05-15 (generated)
* @date			2019-05-13 (last modified)
* @pre			Indizes of []-operator run from 0 to dataSize-1;
* @copyright	This file is part of Exudyn. Exudyn is free software: you can redistribute it and/or modify it under the terms of the Exudyn license. See 'LICENSE.txt' for more details.
* @note			Bug reports, support and further information:
* 				- email: johannes.gerstmayr@uibk.ac.at
* 				- weblink: missing
* 				
*
* *** Example code ***
*
* @code{.cpp}
* ConstSizeMatrix<50> m(5, 5);			//create a matrix with 25 Real entries, allocated 50 entries
* for (Index i = 0; i < 5; i++) {
*   for (Index j = 0; j < 5; j++) {
*     m(i,j) = i*j;
* }
* ConstSizeMatrix<25> m2 = m1;	//assign m1 to m2; m2 has different max size than m1
* m1 += m2;					    //add m2 to m1
* cout << v1 << "\n";			//write m1 to cout
* @endcode
************************************************************************************************ */
#ifndef CONSTSIZEMATRIX__H
#define CONSTSIZEMATRIX__H

#include <initializer_list> //for initializer_list in constructor
#include <ostream>          //ostream for matrix output as text
#include <iostream>         //for cout @todo remove cout from class matrix ==> add Error handling

#include "Utilities/ReleaseAssert.h"
#include "Utilities/BasicDefinitions.h"
#include "Linalg/Matrix.h"

template <typename T, Index dataSize>
class ConstSizeMatrixBase : public MatrixBase<T>
{
private:
	T constData[dataSize];
public:

	//Constructors, Destructor

	//! create empty (0 x 0) matrix
	ConstSizeMatrixBase()
	{
		this->data = &constData[0]; //constructor should always start with data linkage
									//use this-> to access member variables, because of templated base class
		this->numberOfRows = 0;
		this->numberOfColumns = 0;
	}

    //! create matrix with dimensions numberOfRowsInit x numberOfColumnsInit; data is not initialized
	ConstSizeMatrixBase(Index numberOfRowsInit, Index numberOfColumnsInit)
    {
		this->data = &constData[0];
		
		CHECKandTHROW((numberOfRowsInit >= 0 && numberOfColumnsInit >= 0 && numberOfRowsInit*numberOfColumnsInit <= dataSize),
            "ConstSizeMatrixBase::ConstSizeMatrixBase(Index, Index): invalid parameters");

        ResizeMatrix(numberOfRowsInit, numberOfColumnsInit);
    }

	//! @todo disable copy/move constructors/operators: NotCopyableOrMovable(NotCopyableOrMovable&&) = delete;
    //! NotCopyableOrMovable& operator=(NotCopyableOrMovable&&) = delete;
    //! Copyable(const Copyable& rhs) = default;
    //! Copyable& operator=(const Copyable& rhs) = default;

    //! create matrix with dimensions numberOfRowsInit x numberOfColumnsInit; initialize items with 'initializationValue'
	ConstSizeMatrixBase(Index numberOfRowsInit, Index numberOfColumnsInit, T initializationValue)
    {
		this->data = &constData[0];
		
		CHECKandTHROW((numberOfRowsInit >= 0 && numberOfColumnsInit >= 0 && numberOfRowsInit*numberOfColumnsInit <= dataSize),
            "ConstSizeMatrixBase::ConstSizeMatrixBase(Index, Index, T): invalid parameters");
		CHECKandTHROW((initializationValue == 0), "ConstSizeMatrixBase: initializationValue != 0"); 

        ResizeMatrix(numberOfRowsInit, numberOfColumnsInit);

        for (auto &item : *this) {
            item = initializationValue;
        }
    }

    //! create matrix with dimensions numberOfRowsInit x numberOfColumnsInit; initialize data with initializer list
	ConstSizeMatrixBase(Index numberOfRowsInit, Index numberOfColumnsInit, std::initializer_list<T> listOfReals)
    {
		this->data = &constData[0];
		
		CHECKandTHROW((numberOfRowsInit >= 0 && numberOfColumnsInit >= 0 &&
                        numberOfRowsInit*numberOfColumnsInit == listOfReals.size()),
                       "ConstSizeMatrixBase::ConstSizeMatrixBase(Index, Index, initializer_list): inconsistent size of initializer_list");

        ResizeMatrix(numberOfRowsInit, numberOfColumnsInit);

        Index cnt = 0;
        for (auto value : listOfReals) {
			this->data[cnt++] = value;
        }
    }

    //! copy constructor, based on iterators
	ConstSizeMatrixBase(const ConstSizeMatrixBase& matrix)
    {
		this->data = &constData[0];
		
		CHECKandTHROW((matrix.numberOfRows*matrix.numberOfColumns <= dataSize),
			"ConstSizeMatrixBase::ConstSizeMatrixBase(const ConstSizeMatrixBase& matrix): dataSize too small");
		
		ResizeMatrix(matrix.NumberOfRows(), matrix.NumberOfColumns());

        Index cnt = 0;
        for (auto value : matrix) { this->data[cnt++] = value;}
    }

	//! copy constructor with MatrixBase; @todo: check if ConstSizeMatrixBase(const MatrixBase& matrix) is NEEDED?
	ConstSizeMatrixBase(const MatrixBase<T>& matrix)
	{
		this->data = &constData[0];

		CHECKandTHROW((matrix.NumberOfRows()*matrix.NumberOfColumns() <= dataSize),
			"ConstSizeMatrixBase::ConstSizeMatrixBase(const MatrixBase& matrix): dataSize too small");

		ResizeMatrix(matrix.NumberOfRows(), matrix.NumberOfColumns());

		Index cnt = 0;
		for (auto value : matrix) { this->data[cnt++] = value; }
	}

	//! copy from std::array<std::array<Real,nCols>,nRows> matrix
	template<Index matrixColumns, Index matrixRows>
	ConstSizeMatrixBase(const std::array<std::array<T, matrixColumns>, matrixRows>& matrix)
	{
		this->data = &constData[0];

		CHECKandTHROW((matrixRows*matrixColumns <= dataSize),
			"ConstSizeMatrixBase::ConstSizeMatrixBase(const std::array<std::array<T, matrixColumns>, matrixRows>& matrix): dataSize too small");

		ResizeMatrix(matrixRows, matrixColumns);

		Index cnt = 0;
		for (Index rows = 0; rows < matrixRows; rows++)
		{
			for (Index cols = 0; cols < matrixColumns; cols++)
			{
				constData[cnt++] = matrix[rows][cols];
			}
		}
	}


	virtual ~ConstSizeMatrixBase<T, dataSize>()
    {
		this->data = nullptr; //MatrixBase destructor called hereafter
		//do nothing, because no memory allocated
    };


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // BASIC FUNCTIONS
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

protected:
	virtual Index MaxAllocatedSize() const { return dataSize; }
	virtual bool IsConstSizeMatrix() const  override { return true; }

	//! Initialization of data ==> should never be called in ConstSizeMatrixBase; @todo: delete after tests finished
	virtual void Init() override
	{
		this->numberOfRows = 0;
		this->numberOfColumns = 0;
		CHECKandTHROWstring("ConstSizeMatrixBase::Init(): should never be called");
	};

    //! Set new size of matrix; for external access, use 'SetNumberOfRowsAndColumns' to modify size of matrix
    virtual void ResizeMatrix(Index numberOfRowsInit, Index numberOfColumnsInit) override
    {
		CHECKandTHROW((numberOfRowsInit*numberOfColumnsInit <= dataSize),
			"ConstSizeMatrixBase::ResizeMatrix(Index,Index): dataSize too small");
		
		this->numberOfRows = numberOfRowsInit;
		this->numberOfColumns = numberOfColumnsInit;
    }

public:

	//! add two matrices m1 and m2 (for each component); creates new ConstSizeMatrixBase / no memory allocation
	friend ConstSizeMatrixBase<T,dataSize> operator+ (const ConstSizeMatrixBase& m1, const ConstSizeMatrixBase& m2)
	{
		CHECKandTHROW(m1.NumberOfColumns() == m2.NumberOfColumns() && m1.NumberOfRows() == m2.NumberOfRows(),
			"operator+(ConstSizeMatrixBase,ConstSizeMatrixBase): Size mismatch");
		CHECKandTHROW((m1.NumberOfColumns()*m1.NumberOfRows() <= dataSize),
			"ConstSizeMatrixBase::operator+(const ConstSizeMatrixBase&, const ConstSizeMatrixBase&): dataSize too small");

		ConstSizeMatrixBase<T, dataSize> result(m1.NumberOfColumns(), m1.NumberOfRows());
		Index cnt = 0;
		for (auto &item : result)
		{
			item = m1.GetItem(cnt) + m2.GetItem(cnt);
			cnt++; //do not put cnt++ in above line, because order of summation may be interchanged ==> wrong ++ !!!
		}
		return result;
	}

	//! subtract matrix m2 from m1 (for each component); creates new ConstSizeMatrixBase / no memory allocation
	friend ConstSizeMatrixBase<T, dataSize> operator- (const ConstSizeMatrixBase& m1, const ConstSizeMatrixBase& m2)
	{
		CHECKandTHROW(m1.NumberOfColumns() == m2.NumberOfColumns() && m1.NumberOfRows() == m2.NumberOfRows(),
			"operator+(ConstSizeMatrixBase,ConstSizeMatrixBase): Size mismatch");
		CHECKandTHROW((m1.NumberOfColumns()*m1.NumberOfRows() <= dataSize),
			"ConstSizeMatrixBase::operator+(const ConstSizeMatrixBase&, const ConstSizeMatrixBase&): dataSize too small");

		ConstSizeMatrixBase<T, dataSize> result(m1.NumberOfColumns(), m1.NumberOfRows());
		Index cnt = 0;
		for (auto &item : result)
		{
			item = m1.GetItem(cnt) - m2.GetItem(cnt);
			cnt++; //do not put cnt++ in above line, because order of summation may be interchanged ==> wrong ++ !!!
		}
		return result;
	}

    //! multiply matrix m1*m2 (matrix multiplication); algorithm has order O(n^3); creates new ConstSizeMatrixBase / no memory allocation
    friend ConstSizeMatrixBase<T, dataSize> operator* (const ConstSizeMatrixBase& m1, const ConstSizeMatrixBase& m2)
    {
        CHECKandTHROW(m1.NumberOfColumns() == m2.NumberOfRows(),
            "operator*(ConstSizeMatrixBase,ConstSizeMatrixBase): Size mismatch");
		CHECKandTHROW((m1.NumberOfRows()*m2.NumberOfColumns() <= dataSize),
			"ConstSizeMatrixBase::operator*(const ConstSizeMatrixBase&, const ConstSizeMatrixBase&): dataSize too small");

        ConstSizeMatrixBase<T, dataSize> result(m1.NumberOfRows(), m2.NumberOfColumns());

        for (Index i = 0; i < m2.NumberOfColumns(); i++) 
        {
            for (Index j = 0; j < m1.NumberOfRows(); j++)
            {
                T value = 0;
                for (Index k = 0; k < m1.NumberOfColumns(); k++)
                {
                    value += m1(j, k)*m2(k,i);
                }
                result(j,i) = value;
            }
        }
        return result;
    }

    //! multiply matrix with scalar value; creates new ConstSizeMatrixBase / no memory allocation
    friend ConstSizeMatrixBase operator* (const ConstSizeMatrixBase& matrix, const T& value)
    {
        ConstSizeMatrixBase<T, dataSize> result = matrix;
        result *= value;
        return result;
    }

    //! multiply scalar value with matrix; creates new ConstSizeMatrixBase / no memory allocation
    friend ConstSizeMatrixBase operator* (const T& value, const ConstSizeMatrixBase& matrix)
    {
		ConstSizeMatrixBase<T, dataSize> result = matrix;
		result *= value;
		return result;
	}

	//does not work!!!
	//template<class TVector, Index dataSize2>
	//friend SlimVectorBase<T, dataSize2> operator*(const ConstSizeMatrixBase& matrix, const TVector& vector)

	//leads to problems, would also be used for 4x3 matrices!!!
	//friend SlimVectorBase<T, 3> operator*(const ConstSizeMatrixBase& matrix, const SlimVectorBase<T, 3>& vector)


    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // EXTENDED FUNCTIONS
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    //! computes and returns the transposed of *this (does not change *this)
    virtual ConstSizeMatrixBase<T, dataSize> GetTransposed() const
    {
        ConstSizeMatrixBase<T, dataSize> result(this->numberOfColumns, this->numberOfRows);

        for (Index i = 0; i < this->numberOfRows; i++) {
            for (Index j = 0; j < this->numberOfColumns; j++) {
                result(j,i) = (*this)(i,j);
            }
        }
        return result;
    }

	//! get fast inverse for 1D, 2D and 3D case
	virtual ConstSizeMatrixBase<T, dataSize> GetInverse() const
	{
		CHECKandTHROW(this->numberOfColumns <= 3 && this->numberOfColumns == this->numberOfRows, "ConstSizeMatrixBase::GetInverse(): only implemented for dimensions (1x1, 2x2 and 3x3)");

		switch (this->numberOfColumns)
		{
			case 1: 
			{
				T x = (*this)(0, 0);
				CHECKandTHROW(x != 0, "Matrix1D::Invert: matrix is singular");
				return ConstSizeMatrixBase<T, dataSize>(1, 1, { (T)1. / x });
				break;
			}
			case 2: 
			{
				//m=[a b; c d]
				//minv = 1/(ad-bc)[d -b; -c a]
				ConstSizeMatrixBase<T, dataSize> result(2, 2);
				T det = ((*this)(0, 0)*(*this)(1, 1) - (*this)(0, 1)*(*this)(1, 0));
				CHECKandTHROW(det != 0, "Matrix2D::Invert: matrix is singular");

				T invdet = (T)1 / det;

				result(0, 0) = invdet * (*this)(1, 1);
				result(0, 1) =-invdet * (*this)(0, 1);
				result(1, 0) =-invdet * (*this)(1, 0);
				result(1, 1) = invdet * (*this)(0, 0);
				return result;
				break;
			}
			case 3:
			{
				const ConstSizeMatrixBase<T, dataSize>& m = *this;
				T det = m(0, 0) * (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) - m(0, 1) * (m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0)) + m(0, 2) * (m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0));
				CHECKandTHROW(det != 0, "Matrix3D::Invert: matrix is singular");

				T invdet = (T)1 / det;

				ConstSizeMatrixBase<T, dataSize> result(3,3); // inverse of matrix m
				result(0, 0) = invdet * (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2));
				result(0, 1) = invdet * (m(0, 2) * m(2, 1) - m(0, 1) * m(2, 2));
				result(0, 2) = invdet * (m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1));
				result(1, 0) = invdet * (m(1, 2) * m(2, 0) - m(1, 0) * m(2, 2));
				result(1, 1) = invdet * (m(0, 0) * m(2, 2) - m(0, 2) * m(2, 0));
				result(1, 2) = invdet * (m(1, 0) * m(0, 2) - m(0, 0) * m(1, 2));
				result(2, 0) = invdet * (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1));
				result(2, 1) = invdet * (m(2, 0) * m(0, 1) - m(0, 0) * m(2, 1));
				result(2, 2) = invdet * (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1));
				return result;
				break;
			}
			default: //may not occur due to static assertion
				return *this;
		}
	}

};

//multiplication must be defined outside and with "9" ConstSizeMatrixBase<T, 9>, otherwise this operator is also used for 4x3 matrices
template<typename T>
SlimVectorBase<T, 3> operator*(const ConstSizeMatrixBase<T, 9>& matrix, const SlimVectorBase<T, 3>& vector)
{
	CHECKandTHROW(matrix.NumberOfColumns() == vector.NumberOfItems(),
		"operator*(ConstSizeMatrixBase,SlimVectorBase<T, 3>): Size mismatch");
	CHECKandTHROW((matrix.NumberOfRows() == 3),
		"operator*(ConstSizeMatrixBase,SlimVectorBase<T, 3>): matrix does not fit");

	SlimVectorBase<T, 3> result; //no initialization for SlimVector

	for (Index i = 0; i < result.NumberOfItems(); i++)
	{
		T resultRow = 0;
		for (Index j = 0; j < vector.NumberOfItems(); j++)
		{
			resultRow += matrix(i, j)*vector[j];
		}
		result[i] = resultRow;
	}
	return result;
}

//multiplication must be defined outside and with "9" ConstSizeMatrixBase<T, 9>, otherwise this operator is also used for 4x3 matrices
template<typename T>
SlimVectorBase<T, 3> operator*(const SlimVectorBase<T, 3>& vector, const ConstSizeMatrixBase<T, 9>& matrix)
{
	CHECKandTHROW(matrix.NumberOfRows() == vector.NumberOfItems(),
		"operator*(SlimVectorBase<T, 3>,ConstSizeMatrixBase): Size mismatch");
	CHECKandTHROW((matrix.NumberOfColumns() == 3),
		"operator*(SlimVectorBase<T, 3>,ConstSizeMatrixBase): matrix does not fit");

	SlimVectorBase<T, 3> result; //no initialization for SlimVector

	for (Index i = 0; i < result.NumberOfItems(); i++)
	{
		T resultRow = 0;
		for (Index j = 0; j < vector.NumberOfItems(); j++)
		{
			resultRow += vector[j] * matrix(j, i);
		}
		result[i] = resultRow;
	}
	return result;
}

//multiplication must be defined outside and with "4" ConstSizeMatrixBase<T, 4>
template<typename T>
SlimVectorBase<T, 2> operator*(const ConstSizeMatrixBase<T, 4>& matrix, const SlimVectorBase<T, 2>& vector)
{
	CHECKandTHROW(matrix.NumberOfColumns() == vector.NumberOfItems(),
		"operator*(ConstSizeMatrixBase,SlimVectorBase<T, 2>): Size mismatch");
	CHECKandTHROW((matrix.NumberOfRows() == 2),
		"operator*(ConstSizeMatrixBase,SlimVectorBase<T, 2>): matrix does not fit");

	SlimVectorBase<T, 2> result; //no initialization for SlimVector

	for (Index i = 0; i < result.NumberOfItems(); i++)
	{
		T resultRow = 0;
		for (Index j = 0; j < vector.NumberOfItems(); j++)
		{
			resultRow += matrix(i, j)*vector[j];
		}
		result[i] = resultRow;
	}
	return result;
}

//multiplication must be defined outside and with "4" ConstSizeMatrixBase<T, 4>
template<typename T>
SlimVectorBase<T, 2> operator*(const SlimVectorBase<T, 2>& vector, const ConstSizeMatrixBase<T, 4>& matrix)
{
	CHECKandTHROW(matrix.NumberOfRows() == vector.NumberOfItems(),
		"operator*(SlimVectorBase<T, 2>,ConstSizeMatrixBase): Size mismatch");
	CHECKandTHROW((matrix.NumberOfColumns() == 2),
		"operator*(SlimVectorBase<T, 2>,ConstSizeMatrixBase): matrix does not fit");

	SlimVectorBase<T, 2> result; //no initialization for SlimVector

	for (Index i = 0; i < result.NumberOfItems(); i++)
	{
		T resultRow = 0;
		for (Index j = 0; j < vector.NumberOfItems(); j++)
		{
			resultRow += vector[j] * matrix(j, i);
		}
		result[i] = resultRow;
	}
	return result;
}


template<Index dataSize>
using ConstSizeMatrix = ConstSizeMatrixBase<Real,dataSize>;

template<Index dataSize>
using ConstSizeMatrixF = ConstSizeMatrixBase<float, dataSize>;

//not yet supported in C++ standard: typedef ConstSizeMatrixBase<Real, Index dataSize> ConstSizeMatrix<dataSize>;

//! matrix*vector multiplication with given result vector; invokes memory allocation
//template<class TVector, Index dataSize, Index dataSize2>
//template <>
//SlimVector<3> operator*(const ConstSizeVector<9>& matrix, const SlimVector<3>& vector)
//{
//	Vector3D result;
//	EXUmath::MultMatrixVector(matrix, vector, result);
//	return result;
//}
//

//{
//	CHECKandTHROW(matrix.NumberOfColumns() == vector.NumberOfItems(),
//		"operator*(Matrix,TVector): Size mismatch");
//	CHECKandTHROW((matrix.NumberOfRows() <= dataSize),
//		"operator*(Matrix,TVector): dataSize too small");
//
//	ConstSizeVector<dataSize> result(matrix.NumberOfRows());
//
//	for (Index i = 0; i < result.NumberOfItems(); i++)
//	{
//		double resultRow = 0;
//		for (Index j = 0; j < vector.NumberOfItems(); j++)
//		{
//			resultRow += matrix(i, j)*vector[j];
//		}
//		result[i] = resultRow;
//	}
//	return result;
//}

#endif

