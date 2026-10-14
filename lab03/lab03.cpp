#include <iostream>
#include <utility>
using namespace std;

struct Size {
    int rows;
    int cols;
};

bool operator==(const Size &first, const Size &second) {
    return first.rows == second.rows && first.cols == second.cols;
}

bool operator!=(const Size &first, const Size &second) { return !(first == second); }

class Matrix {
  private:
    int rows_;
    int cols_;
    double *data_;
    bool owns_;

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
                transposed.SetElement(j, i, (*this)(i, j));
            }
        }

        return transposed;
    }

    // double
    operator double() const {
        if (rows_ == 1 && cols_ == 1) {
            return data_[0];
        }
        throw invalid_argument("Matrix is not 1x1");
    }

    // []
    Matrix operator[](int i) const {
        if (rows_ == 1) {
            return Matrix(1, 1, data_ + (i - 1));
        }

        return Matrix(1, cols_, data_ + (i - 1) * cols_);
    }

    // double operator[](int i, int j) const { return (*this)(i, j); }

    // ()
    double operator()(int i, int j) const { return GetElement(i, j); }

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
        int rows = Rows();
        int cols = other.Cols();
        if (cols != rows) {
            return *this;
        }

        Matrix tmp(rows, cols);

        for (int i = 1; i <= rows; ++i) {
            for (int j = 1; j <= cols; ++j) {

                double c_ij = 0.0;

                for (int r = 1; r <= cols; ++r) {
                    c_ij += (*this)(i, r) * other(r, j);
                }

                tmp.SetElement(i, j, c_ij);
            }
        }

        *this = move(tmp);
        return *this;
    }

    // *= scalar
    Matrix &operator*=(const double scalar) {
        for (int i = 0; i < rows_ * cols_; ++i) {
            data_[i] *= scalar;
        }
        return *this;
    }

    Matrix LPad(int rows, int cols, double value) const {
        int minRows = min(rows, rows_);
        int minCols = min(cols, cols_);

        Matrix res(rows, cols, value);

        for (int i = 0; i < minRows; ++i) {
            copyData(data_ + i * cols_, res.data_ + i * cols, minCols);
        }

        return res;
    }
    Matrix &LPad(int rows, int cols, double value) {
        int minRows = min(rows, rows_);
        int minCols = min(cols, cols_);

        Matrix res(rows, cols, value);

        for (int i = 0; i < minRows; ++i) {
            copyData(data_ + i * cols_, res.data_ + i * cols, minCols);
        }

        return *this = move(res);
    }

    // ++
    Matrix &operator++() { return LPad(rows_ + 1, cols_ + 1, 0.0); }

    // --
    Matrix &operator--() {
        // if (rows_ <= 1 || cols_ <= 1) {
        //     return *this;
        // }
        return LPad(rows_ - 1, cols_ - 1, 0.0);
    }

    Matrix RPad(int rows, int cols, double value) const {
        int minRows = min(rows, rows_);
        int minCols = min(cols, cols_);

        Matrix res(rows, cols, value);

        for (int i = 0; i < minRows; ++i) {
            int offset = (i + (rows - minRows)) * cols + (cols - minCols);
            copyData(data_ + i * cols_, res.data_ + offset, minCols);
        }

        return res;
    }

    Matrix &RPad(int rows, int cols, double value) {
        int minRows = min(rows, rows_);
        int minCols = min(cols, cols_);

        Matrix res(rows, cols, value);

        int srcRowShift = rows_ - minRows;
        int srcColShift = cols_ - minCols;

        for (int i = 0; i < minRows; ++i) {
            const double *src = data_ + (i + srcRowShift) * cols_ + srcColShift;
            double *dst = res.data_ + (i + (rows - minRows)) * cols + (cols - minCols);
            copyData(src, dst, minCols);
        }

        return *this = move(res);
    }

    // ++
    Matrix operator++(int) {
        Matrix old = *this;
        RPad(rows_ + 1, cols_ + 1, 0.0);
        return old;
    }

    // --
    Matrix operator--(int) {
        Matrix old = *this;
        RPad(rows_ - 1, cols_ - 1, 0.0);
        return old;
    }

    Matrix LPad(int rows, int cols) const { return LPad(rows, cols, 0.0); }
    Matrix RPad(int rows, int cols) const { return RPad(rows, cols, 0.0); }

    // Matrix DiagLPad(int rows, int cols, double value) const {
    //     if (rows < rows_ || cols < cols_) {
    //         return Matrix();
    //     }

    //     Matrix res(rows, cols, 0.0);
    //     int n = min(rows, cols);
    //     for (int i = 1; i <= n; ++i) {
    //         res.SetElement(i, i, value);
    //     }

    //     return res;
    // }

    // Matrix DiagRPad(int rows, int cols, double value) const {
    //     if (rows < rows_ || cols < cols_) {
    //         return Matrix();
    //     }

    //     Matrix res(rows, cols, 0.0);
    //     int n = min(rows, cols);
    //     for (int i = 1; i <= n; ++i) {
    //         res.SetElement(rows - n + i, cols - n + i, value);
    //     }

    //     return res;
    // }

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

    void SetElement(int i, int j, double value) {
        if (i < 1 || j < 1 || i > rows_ || j > cols_) {
            throw out_of_range("Index out of bounds");
        }

        data_[(i - 1) * cols_ + (j - 1)] = value;
    }

    friend ostream &operator<<(ostream &os, const Matrix &other);
    friend istream &operator>>(istream &is, Matrix &other);
    friend bool operator==(const Matrix &first, const Matrix &second);
    friend bool operator!=(const Matrix &first, const Matrix &second);
    friend Matrix operator+(const Matrix &first, const Matrix &second);
    friend Matrix operator*(const Matrix &first, const Matrix &second);
};

ostream &operator<<(ostream &out, const Size &size) {
    out << size.rows << "x" << size.cols;
    return out;
}

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
    if (n < 0) {
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
    Matrix m1(2, 3, 8.0);
    cout << "m1" << m1;
    m1.SetElement(2, 3, 5.0);
    cout << "m1 (set)" << m1;

    Matrix m2 = Matrix(3, 2, 1.0);
    cout << "m2" << m2;

    // Matrix m3;
    // cin >> m3;
    // cout << "m3" << m3;
    Matrix m3(2, 3, 4.0);
    cout << "m3" << m3;

    Matrix m4 = m1 + m3;
    cout << "m4 (+)" << m4;

    m4 += m3;
    cout << "m4 (+=)" << m4;

    Matrix m5 = m1 * m2;
    cout << "m5 (*)" << m5;

    const Matrix m6(1.0);
    Matrix m7 = m6.LPad(3, 3, 0.0);
    cout << "m6" << m6;
    cout << "m7 (LPad)" << m7;
    Matrix reverse_m7_lpad = m7.LPad(1, 1, 0.0);
    cout << "reverse_m7_lpad (LPad)" << reverse_m7_lpad;

    m7 = m6.RPad(3, 3, 0.0);
    cout << "m7 (RPad)" << m7;
    Matrix reverse_m7_rpad = m7.RPad(1, 1, 0.0);
    cout << "reverse_m7_rpad (RPad)" << reverse_m7_rpad;

    // m7 = m6.DiagLPad(3, 4, 1.0);
    // cout << "m7 (DiagPad)" << m7;

    double num = m7[2][2];
    cout << "m7[2][2] value = " << num << endl;

    Matrix row = m7[2];
    cout << "m7[2] row" << row;

    cout << "----------" << endl;
    Matrix A(3, 3, 0.0);

    int value = 1;

    for (int i = 1; i <= 3; ++i) {
        for (int j = 1; j <= 3; ++j) {
            A.SetElement(i, j, value++);
        }
    }

    cout << "A:" << A;

    cout << "A[2]:" << A[2];

    cout << "A[2][2] = " << static_cast<double>(A[2][2]) << endl;
    cout << "----------" << endl;

    Matrix m8(2, 2, 2.0);
    Matrix m8_pow = m8 ^ 3;
    cout << "m8:" << m8;
    cout << "m8 ^ 3:" << m8_pow;
    cout << "----------" << endl;

    Matrix m9(2, 2, 3.0);
    cout << "m9:" << m9;
    cout << "m9++:" << m9++;
    cout << "++m9:" << ++m9;

    cout << "m9--:" << m9--;
    cout << "--m9:" << --m9;
    return 0;
}
