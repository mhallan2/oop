#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>
using namespace std;

// Size
struct Size {
    int rows;
    int cols;
};

bool operator==(const Size &first, const Size &second) {
    return first.rows == second.rows && first.cols == second.cols;
}

bool operator!=(const Size &first, const Size &second) { return !(first == second); }

ostream &operator<<(ostream &out, const Size &size) {
    out << size.rows << "x" << size.cols;
    return out;
}

// Matrix
class Matrix {

  private:
    int rows_;
    int cols_;
    double *data_;
    bool owns_;

    void CheckCols(int col) const {
        if (col < 1 || col > cols_) {
            throw out_of_range("Column index out of bounds");
        }
    }

    void CheckRows(int row) const {
        if (row < 1 || row > rows_) {
            throw out_of_range("Row index out of bounds");
        }
    }

    static void copyData(const double *source, double *destination, int size) {
        for (int i = 0; i < size; ++i) {
            destination[i] = source[i];
        }
    }

    static void FillData(double *data, int size, double value) {
        for (int i = 0; i < size; ++i) {
            data[i] = value;
        }
    }

  public:
    // Row class for non-const Matrix
    class Row {

      private:
        double *data_;
        int length_;

      public:
        Row(double *data, int cols) : data_(data), length_(cols) {}
        Row(const Row &other) : data_(other.data_), length_(other.length_) {}

        double &operator[](int col) {
            if (col < 1 || col > length_) {
                throw out_of_range("Column index out of bounds");
            }

            return data_[col - 1];
        }
        const double &operator[](int col) const {
            if (col < 1 || col > length_) {
                throw out_of_range("Column index out of bounds");
            }

            return data_[col - 1];
        }

        int Size() const { return length_; }

        operator Matrix() const;
    };

    class ConstRow {
      private:
        const double *data_;
        int length_;

      public:
        ConstRow(const double *data, int cols) : data_(data), length_(cols) {}

        const double &operator[](int col) const {
            if (col < 1 || col > length_) {
                throw out_of_range("Column index out of bounds");
            }

            return data_[col - 1];
        }

        int Size() const { return length_; }

        operator Matrix() const;
    };

    Row operator[](int row) {
        CheckRows(row);
        return Row(data_ + (row - 1) * cols_, cols_);
    }

    ConstRow operator[](int row) const {
        CheckRows(row);
        return ConstRow(data_ + (row - 1) * cols_, cols_);
    }

    // Constructors
    Matrix() : rows_(0), cols_(0), data_(nullptr), owns_(true) {}

    explicit Matrix(double value) : rows_(1), cols_(1), data_(new double[1]{value}), owns_(true) {}

    Matrix(int rows, int cols, double value) : rows_(0), cols_(0), data_(nullptr), owns_(true) {

        if (rows <= 0 || cols <= 0)
            return;

        rows_ = rows;
        cols_ = cols;

        data_ = new double[rows_ * cols_];
        FillData(data_, rows_ * cols_, value);
    }

    Matrix(int rows, int cols) : Matrix(rows, cols, 0.0) {}

    Matrix(int rows, int cols, double *data)
        : rows_(rows), cols_(cols), data_(data), owns_(false) {}

    // Rule of Five
    // 1
    ~Matrix() {
        if (owns_) {
            delete[] data_;
        }
    }

    // 2
    Matrix(const Matrix &other)
        : rows_(other.rows_), cols_(other.cols_), data_(nullptr), owns_(true) {

        if (other.rows_ == 0 || other.cols_ == 0)
            return;

        data_ = new double[rows_ * cols_];
        copyData(other.data_, data_, rows_ * cols_);
    }

    // 3
    Matrix(Matrix &&other) noexcept
        : rows_(other.rows_), cols_(other.cols_), data_(other.data_), owns_(other.owns_) {

        other.rows_ = other.cols_ = 0;
        other.data_ = nullptr;
        other.owns_ = true;
    }

    // 4
    Matrix &operator=(const Matrix &other) {

        if (this == &other) {
            return *this;
        }

        if (owns_) {
            delete[] data_;
        }

        rows_ = other.rows_;
        cols_ = other.cols_;
        data_ = nullptr;
        owns_ = true;

        if (rows_ != 0 && cols_ != 0) {
            data_ = new double[rows_ * cols_];
            copyData(other.data_, data_, rows_ * cols_);
        }

        return *this;
    }

    // 5
    Matrix &operator=(Matrix &&other) noexcept {

        if (this == &other) {
            return *this;
        }

        if (owns_) {
            delete[] data_;
        }

        data_ = other.data_;
        rows_ = other.rows_;
        cols_ = other.cols_;
        owns_ = other.owns_;

        other.data_ = nullptr;
        other.rows_ = other.cols_ = 0;
        other.owns_ = true;

        return *this;
    }

    Matrix T() const {
        Matrix transposed(cols_, rows_);

        for (int i = 1; i <= rows_; ++i) {
            for (int j = 1; j <= cols_; ++j) {
                transposed(i, j) = (*this)(j, i);
            }
        }

        return transposed;
    }

    // ()
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

    // +=
    Matrix &operator+=(const Matrix &other) {
        if (GetSize() != other.GetSize()) {
            return *this;
        }

        for (int i = 0; i < rows_ * cols_; ++i) {
            data_[i] += other.data_[i];
        }

        return *this;
    }

    // -=
    Matrix &operator-=(const Matrix &other) {
        if (GetSize() != other.GetSize()) {
            return *this;
        }

        for (int i = 0; i < rows_ * cols_; ++i) {
            data_[i] -= other.data_[i];
        }

        return *this;
    }

    // *=
    Matrix &operator*=(const Matrix &other) {
        int m = rows_;
        int n = cols_;
        int k = other.cols_;

        if (n != other.rows_) {
            throw invalid_argument("Matrix dimensions mismatch for *=");
        }

        Matrix tmp(m, k);

        for (int i = 1; i <= m; ++i) {
            for (int j = 1; j <= k; ++j) {
                double c_ij = 0.0;
                for (int r = 1; r <= n; ++r) {
                    c_ij += (*this)(i, r) * other(r, j);
                }
                tmp.SetElement(i, j, c_ij);
            }
        }

        return *this = move(tmp);
    }

    // *= scalar
    Matrix &operator*=(const double scalar) {
        for (int i = 0; i < rows_ * cols_; ++i) {
            data_[i] *= scalar;
        }
        return *this;
    }

    Matrix LPad(int rows, int cols, double value) const {
        if (rows <= 0 || cols <= 0)
            throw invalid_argument("bad size");

        int minRows = min(rows, rows_);
        int minCols = min(cols, cols_);

        Matrix res(rows, cols, value);

        for (int i = 0; i < minRows; ++i) {
            copyData(data_ + i * cols_, res.data_ + i * cols, minCols);
        }

        return res;
    }

    Matrix RPad(int rows, int cols, double value) const {
        if (rows < 0 || cols < 0)
            throw invalid_argument("Bad matrix size");

        Matrix result(rows, cols, value);

        int copiedRows = min(rows, rows_);
        int copiedCols = min(cols, cols_);

        int sourceRow = rows_ - copiedRows;
        int sourceCol = cols_ - copiedCols;

        int destinationRow = rows - copiedRows;
        int destinationCol = cols - copiedCols;

        for (int i = 0; i < copiedRows; ++i) {
            copyData(data_ + (sourceRow + i) * cols_ + sourceCol,
                     result.data_ + (destinationRow + i) * cols + destinationCol, copiedCols);
        }

        return result;
    }

    Matrix LPad(int rows, int cols) const { return LPad(rows, cols, 0.0); }
    Matrix RPad(int rows, int cols) const { return RPad(rows, cols, 0.0); }

    // ++
    Matrix operator++() {
        *this = LPad(rows_ + 1, cols_ + 1, 0.0);
        return *this;
    }

    // --
    Matrix operator--() {
        *this = LPad(rows_ - 1, cols_ - 1, 0.0);
        return *this;
    }

    // ++
    Matrix operator++(int) {
        *this = RPad(rows_ + 1, cols_ + 1, 0.0);
        return *this;
    }

    // --
    Matrix operator--(int) {
        *this = RPad(rows_ - 1, cols_ - 1, 0.0);
        return *this;
    }

    // Getters
    Size GetSize() const { return {rows_, cols_}; }

    int Rows() const { return rows_; }

    int Cols() const { return cols_; }

    bool OwnsMemory() const { return owns_; }

    double GetElement(int i, int j) const {
        if (i < 1 || j < 1 || i > rows_ || j > cols_) {
            throw out_of_range("Index out of bounds");
        }

        return data_[(i - 1) * cols_ + (j - 1)];
    }

    // Setters
    void SetElement(int i, int j, double value) {
        if (i < 1 || j < 1 || i > rows_ || j > cols_) {
            throw out_of_range("Index out of bounds");
        }

        data_[(i - 1) * cols_ + (j - 1)] = value;
    }

    // Friend functions
    friend ostream &operator<<(ostream &os, const Matrix &other);
    friend istream &operator>>(istream &is, Matrix &other);
    friend bool operator==(const Matrix &first, const Matrix &second);
    friend bool operator!=(const Matrix &first, const Matrix &second);
};

// Matrix conversion
Matrix::Row::operator Matrix() const {
    Matrix result(1, length_);

    for (int col = 1; col <= length_; ++col) {
        result.SetElement(1, col, data_[col - 1]);
    }

    return result;
}

Matrix::ConstRow::operator Matrix() const {
    Matrix result(1, length_);

    for (int col = 1; col <= length_; ++col) {
        result.SetElement(1, col, data_[col - 1]);
    }

    return result;
}

// <<
ostream &operator<<(ostream &out, const Matrix &matrix) {
    Size size = matrix.GetSize();
    out << " (size=" << size << ", owns=" << boolalpha << matrix.OwnsMemory() << "):\n";

    out << "[";
    for (int i = 1; i <= matrix.rows_; ++i) {

        out << "[";

        for (int j = 1; j <= matrix.cols_; ++j) {
            if (j != 1) {
                out << ", ";
            }
            out << matrix(i, j);
        }

        out << "]";

        if (i != matrix.rows_) {
            out << ",\n ";
        }
    }

    out << "]\n";
    return out;
}

// >>
istream &operator>>(istream &in, Matrix &matrix) {
    int rows;
    int cols;

    in >> rows >> cols;
    Matrix m(rows, cols);
    for (int i = 1; i <= rows; ++i) {
        for (int j = 1; j <= cols; ++j) {
            double value;
            in >> value;
            m.SetElement(i, j, value);
        }
    }
    matrix = move(m);
    return in;
}

// ==
bool operator==(const Matrix &first, const Matrix &second) {
    if (first.GetSize() != second.GetSize()) {
        return false;
    }

    for (int i = 1; i <= first.rows_; ++i) {
        for (int j = 1; j <= first.cols_; ++j) {
            if (first(i, j) != second(i, j)) {
                return false;
            }
        }
    }

    return true;
}

// !=
bool operator!=(const Matrix &first, const Matrix &second) { return !(first == second); }

// +
Matrix operator+(const Matrix &first, const Matrix &second) {
    Matrix sum = first;
    return sum += second;
}

// *
Matrix operator*(const Matrix &first, const Matrix &second) {
    Matrix res = first;
    return res *= second;
}

// ^
Matrix operator^(const Matrix &matrix, int n) {
    if (n <= 0) {
        throw invalid_argument("Exponent must be non-negative");
    }

    Matrix res = matrix;

    for (int i = 1; i < n; i++) {
        res *= matrix;
    }

    return res;
}

// * scalar
Matrix operator*(const Matrix &matrix, const double scalar) {
    Matrix res = matrix;
    return res *= scalar;
}
Matrix operator*(const double scalar, const Matrix &matrix) { return matrix * scalar; }

// -
Matrix operator-(const Matrix &first, const Matrix &second) {
    Matrix sum = first;
    return sum -= second;
}

int main() {
    Matrix A(2, 3);

    // Цепочечная индексация: запись и чтение.
    A[1][1] = 11.0;
    A[1][2] = 12.0;
    A[1][3] = 13.0;
    A[2][1] = 21.0;
    A[2][2] = 22.0;
    A[2][3] = 23.0;

    cout << "A after A[i][j] assignments:" << A;

    // Доступ через operator(i, j).
    cout << "A(1, 2) = " << A(1, 2) << endl;
    cout << "A(2, 3) = " << A(2, 3) << endl;

    A(1, 2) = 120.0;
    cout << "A after A(1, 2) = 120:" << A;

    // Проверка чтения через const Matrix.
    const Matrix B = A;
    cout << "B[1][2] = " << B[1][2] << endl;
    cout << "B(2, 3) = " << B(2, 3) << endl;
    Matrix row = B[1];
    cout << "row = B[1]:" << row;
    // B[1][2] = 100.0; // Ошибка компиляции: B - константная матрица.

    // Вывод строк через преобразование Row/ConstRow -> Matrix.
    cout << "First row of A:" << A[1];
    cout << "Second row of B:" << B[2];

    // Проверка границ индексов.
    try {
        A[3][1] = 0.0;
    } catch (const out_of_range &error) {
        cout << "Invalid row: " << error.what() << endl;
    }

    try {
        A[1][4] = 0.0;
    } catch (const out_of_range &error) {
        cout << "Invalid column: " << error.what() << endl;
    }

    return 0;
}
