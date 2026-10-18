#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>

using namespace std;

struct Size {
    int rows;
    int cols;
};

bool operator==(const Size &left, const Size &right) {
    return left.rows == right.rows && left.cols == right.cols;
}

bool operator!=(const Size &left, const Size &right) { return !(left == right); }

ostream &operator<<(ostream &out, const Size &size) {
    out << size.rows << "x" << size.cols;
    return out;
}

class Matrix {
  private:
    int rows_;
    int cols_;
    double *data_;
    bool owns_;

    void CheckRows(int row) const {
        if (row < 1 || row > rows_) {
            throw out_of_range("Row index out of bounds");
        }
    }

    void CheckCols(int col) const {
        if (col < 1 || col > cols_) {
            throw out_of_range("Column index out of bounds");
        }
    }

    static void CopyData(const double *source, double *destination, int size) {
        for (int i = 0; i < size; ++i) {
            destination[i] = source[i];
        }
    }

    static void FillData(double *data, int size, double value) {
        for (int i = 0; i < size; ++i) {
            data[i] = value;
        }
    }

    class Row {
      private:
        Matrix &matrix_;
        int index_;

        Row(Matrix &matrix, int index) : matrix_(matrix), index_(index) {}

        friend class Matrix;

      public:
        // A[i][j] — доступ к элементу строки i.
        double &operator[](int col) {
            matrix_.CheckRows(index_);
            matrix_.CheckCols(col);
            return matrix_.data_[(index_ - 1) * matrix_.cols_ + (col - 1)];
        }

        // Matrix row = A[i];
        operator Matrix() const;

        // Позволяет писать row[1] = value, если row имеет размер 1 x N.
        Row &operator=(double value) {
            if (matrix_.rows_ != 1) {
                throw invalid_argument("Assignment through operator[] requires a 1-row matrix");
            }

            matrix_.CheckCols(index_);
            matrix_.data_[index_ - 1] = value;
            return *this;
        }
    };

    // Прокси для константной матрицы.
    class ConstRow {
      private:
        const Matrix &matrix_;
        int index_;

        ConstRow(const Matrix &matrix, int index) : matrix_(matrix), index_(index) {}

        friend class Matrix;

      public:
        const double &operator[](int col) const {
            matrix_.CheckRows(index_);
            matrix_.CheckCols(col);
            return matrix_.data_[(index_ - 1) * matrix_.cols_ + (col - 1)];
        }

        // Matrix row = A[i];
        operator Matrix() const;
    };

  public:
    // Неконстантная матрица возвращает изменяемый прокси.
    Row operator[](int row) {
        CheckRows(row);
        return Row(*this, row);
    }

    // Константная матрица возвращает только читаемый прокси.
    ConstRow operator[](int row) const {
        CheckRows(row);
        return ConstRow(*this, row);
    }

    Matrix() : rows_(0), cols_(0), data_(nullptr), owns_(true) {}

    Matrix(double value) : rows_(1), cols_(1), data_(new double[1]{value}), owns_(true) {}

    Matrix(int rows, int cols, double value) : rows_(0), cols_(0), data_(nullptr), owns_(true) {
        if (rows <= 0 || cols <= 0) {
            return;
        }

        rows_ = rows;
        cols_ = cols;
        data_ = new double[rows_ * cols_];
        FillData(data_, rows_ * cols_, value);
    }

    Matrix(int rows, int cols) : Matrix(rows, cols, 0.0) {}

    Matrix(int rows, int cols, double *data)
        : rows_(rows), cols_(cols), data_(data), owns_(false) {}

    ~Matrix() {
        if (owns_) {
            delete[] data_;
        }
    }

    Matrix(const Matrix &other)
        : rows_(other.rows_), cols_(other.cols_), data_(nullptr), owns_(true) {
        if (rows_ == 0 || cols_ == 0) {
            return;
        }

        data_ = new double[rows_ * cols_];
        CopyData(other.data_, data_, rows_ * cols_);
    }

    Matrix(Matrix &&other) noexcept
        : rows_(other.rows_), cols_(other.cols_), data_(other.data_), owns_(other.owns_) {
        other.rows_ = 0;
        other.cols_ = 0;
        other.data_ = nullptr;
        other.owns_ = true;
    }

    Matrix &operator=(const Matrix &other) {
        if (this == &other) {
            return *this;
        }

        if (owns_) {
            delete[] data_;
        }

        rows_ = other.rows_;
        cols_ = other.cols_;
        owns_ = true;
        data_ = nullptr;

        if (rows_ != 0 && cols_ != 0) {
            data_ = new double[rows_ * cols_];
            CopyData(other.data_, data_, rows_ * cols_);
        }

        return *this;
    }

    Matrix &operator=(Matrix &&other) noexcept {
        if (this == &other) {
            return *this;
        }

        if (owns_) {
            delete[] data_;
        }

        rows_ = other.rows_;
        cols_ = other.cols_;
        data_ = other.data_;
        owns_ = other.owns_;

        other.rows_ = 0;
        other.cols_ = 0;
        other.data_ = nullptr;
        other.owns_ = true;

        return *this;
    }

    double &operator()(int row, int col) {
        CheckRows(row);
        CheckCols(col);
        return data_[(row - 1) * cols_ + (col - 1)];
    }

    const double &operator()(int row, int col) const {
        CheckRows(row);
        CheckCols(col);
        return data_[(row - 1) * cols_ + (col - 1)];
    }

    operator double() const {
        if (rows_ != 1 || cols_ != 1) {
            throw invalid_argument("Matrix must be 1x1 to convert to double");
        }
        return data_[0];
    }

    Size GetSize() const { return {rows_, cols_}; }
    int Rows() const { return rows_; }
    int Cols() const { return cols_; }
    bool OwnsMemory() const { return owns_; }

    double GetElement(int row, int col) const { return (*this)(row, col); }

    void SetElement(int row, int col, double value) { (*this)(row, col) = value; }

    Matrix T() const {
        Matrix result(cols_, rows_);

        for (int row = 1; row <= rows_; ++row) {
            for (int col = 1; col <= cols_; ++col) {
                result(col, row) = (*this)(row, col);
            }
        }

        return result;
    }

    Matrix &operator+=(const Matrix &other) {
        if (GetSize() != other.GetSize()) {
            return *this;
        }

        for (int i = 0; i < rows_ * cols_; ++i) {
            data_[i] += other.data_[i];
        }

        return *this;
    }

    Matrix &operator-=(const Matrix &other) {
        if (GetSize() != other.GetSize()) {
            return *this;
        }

        for (int i = 0; i < rows_ * cols_; ++i) {
            data_[i] -= other.data_[i];
        }

        return *this;
    }

    Matrix &operator*=(const Matrix &other) {
        if (cols_ != other.rows_) {
            throw invalid_argument("Matrix dimensions mismatch for *=");
        }

        Matrix result(rows_, other.cols_);

        for (int row = 1; row <= rows_; ++row) {
            for (int col = 1; col <= other.cols_; ++col) {
                for (int k = 1; k <= cols_; ++k) {
                    result(row, col) += (*this)(row, k) * other(k, col);
                }
            }
        }

        return *this = move(result);
    }

    Matrix &operator*=(double value) {
        for (int i = 0; i < rows_ * cols_; ++i) {
            data_[i] *= value;
        }
        return *this;
    }

    Matrix LPad(int rows, int cols, double value = 0.0) const {
        if (rows <= 0 || cols <= 0) {
            throw invalid_argument("Bad matrix size");
        }

        Matrix result(rows, cols, value);
        int copiedRows = min(rows, rows_);
        int copiedCols = min(cols, cols_);

        for (int row = 0; row < copiedRows; ++row) {
            CopyData(data_ + row * cols_, result.data_ + row * cols, copiedCols);
        }

        return result;
    }

    Matrix RPad(int rows, int cols, double value = 0.0) const {
        if (rows <= 0 || cols <= 0) {
            throw invalid_argument("Bad matrix size");
        }

        Matrix result(rows, cols, value);
        int copiedRows = min(rows, rows_);
        int copiedCols = min(cols, cols_);
        int sourceRow = rows_ - copiedRows;
        int sourceCol = cols_ - copiedCols;
        int destinationRow = rows - copiedRows;
        int destinationCol = cols - copiedCols;

        for (int row = 0; row < copiedRows; ++row) {
            CopyData(data_ + (sourceRow + row) * cols_ + sourceCol,
                     result.data_ + (destinationRow + row) * cols + destinationCol, copiedCols);
        }

        return result;
    }

    Matrix operator++() {
        *this = LPad(rows_ + 1, cols_ + 1);
        return *this;
    }

    Matrix operator--() {
        *this = LPad(rows_ - 1, cols_ - 1);
        return *this;
    }

    Matrix operator++(int) {
        *this = RPad(rows_ + 1, cols_ + 1);
        return *this;
    }

    Matrix operator--(int) {
        *this = RPad(rows_ - 1, cols_ - 1);
        return *this;
    }

    // friend ostream &operator<<(ostream &out, const Matrix &matrix);
    // friend istream &operator>>(istream &in, Matrix &matrix);
};

Matrix::Row::operator Matrix() const {
    matrix_.CheckRows(index_);
    Matrix result(1, matrix_.cols_);

    for (int col = 1; col <= matrix_.cols_; ++col) {
        result(1, col) = matrix_(index_, col);
    }

    return result;
}

Matrix::ConstRow::operator Matrix() const {
    matrix_.CheckRows(index_);
    Matrix result(1, matrix_.cols_);

    for (int col = 1; col <= matrix_.cols_; ++col) {
        result(1, col) = matrix_(index_, col);
    }

    return result;
}

ostream &operator<<(ostream &out, const Matrix &matrix) {
    int rows = matrix.Rows();
    int cols = matrix.Cols();
    out << "(size=" << rows << "x" << cols << ", owns=" << boolalpha << matrix.OwnsMemory()
        << ")\n[";

    for (int row = 1; row <= rows; ++row) {
        if (row != 1) {
            out << ",\n ";
        }

        out << "[";
        for (int col = 1; col <= cols; ++col) {
            if (col != 1) {
                out << ", ";
            }
            out << matrix(row, col);
        }
        out << "]";
    }

    out << "]\n";
    return out;
}

istream &operator>>(istream &in, Matrix &matrix) {
    int rows;
    int cols;
    in >> rows >> cols;

    Matrix result(rows, cols);
    for (int row = 1; row <= rows; ++row) {
        for (int col = 1; col <= cols; ++col) {
            in >> result(row, col);
        }
    }

    matrix = move(result);
    return in;
}

bool operator==(const Matrix &left, const Matrix &right) {
    if (left.GetSize() != right.GetSize()) {
        return false;
    }

    for (int row = 1; row <= left.Rows(); ++row) {
        for (int col = 1; col <= left.Cols(); ++col) {
            if (left(row, col) != right(row, col)) {
                return false;
            }
        }
    }

    return true;
}

bool operator!=(const Matrix &left, const Matrix &right) { return !(left == right); }

Matrix operator+(const Matrix &left, const Matrix &right) {
    Matrix result = left;
    return result += right;
}

Matrix operator-(const Matrix &left, const Matrix &right) {
    Matrix result = left;
    return result -= right;
}

Matrix operator*(const Matrix &left, const Matrix &right) {
    Matrix result = left;
    return result *= right;
}

Matrix operator*(const Matrix &matrix, double value) {
    Matrix result = matrix;
    return result *= value;
}

Matrix operator*(double value, const Matrix &matrix) { return matrix * value; }

Matrix operator^(const Matrix &matrix, int power) {
    if (power <= 0) {
        throw invalid_argument("Power must be positive");
    }

    Matrix result = matrix;
    for (int i = 1; i < power; ++i) {
        result *= matrix;
    }

    return result;
}

int main() {
    const Matrix A(2, 3, 0.0);
    Matrix row1 = A[1];
    row1[1] = 2.0;

    cout << "A:\n" << A;
    cout << "row1:\n" << row1;

    return 0;
}
