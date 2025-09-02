#include <iostream>
#include <string>
#include <algorithm>
using namespace std;
class BigInt {
private:
    string number; // Digits only, no sign
    bool isNegative;

    // Remove leading zeros
    void removeLeadingZeros() {
        while (number.size() > 1 && number[0] == '0') {
            number.erase(0, 1);
        }
        if (number == "0") {
            isNegative = false;
        }
    }
    // Compare absolute values
    int compareMagnitude(const BigInt& other) const {
        if (number.size() != other.number.size()) {
            return (number.size() < other.number.size()) ? -1 : 1;
        }
        if (number == other.number) return 0;
        return (number < other.number) ? -1 : 1;
    }

public:
    // Constructors
    BigInt() : number("0"), isNegative(false) {}
    BigInt(long long num) {
        if (num < 0) {
            isNegative = true;
            num = -num;
        } else {
            isNegative = false;
        }
        number = std::to_string(num);
    }
    BigInt(const std::string& str) {
        if (str.size() > 1 && str[0] == '-') {
            isNegative = true;
            number = str.substr(1);
        } else {
            isNegative = false;
            number = str;
        }
        if (number == "0") isNegative = false;
        removeLeadingZeros();
    }

    // Unary operators
    BigInt operator+() const { return *this; }
    BigInt operator-() const {
        BigInt temp = *this;
        if (temp.number != "0") temp.isNegative = !isNegative;
        return temp;
    }

    // Comparison operators
    bool operator==(const BigInt& other) const {
        return isNegative == other.isNegative && number == other.number;
    }
    bool operator!=(const BigInt& other) const { return !(*this == other); }

    bool operator<(const BigInt& other) const {
        if (isNegative != other.isNegative)
            return isNegative; // negative < positive
        int cmp = compareMagnitude(other);
        return isNegative ? (cmp > 0) : (cmp < 0);
    }
    bool operator>(const BigInt& other) const { return other < *this; }
    bool operator<=(const BigInt& other) const { return !(*this > other); }
    bool operator>=(const BigInt& other) const { return !(*this < other); }

    // Addition
    BigInt& operator+=(const BigInt& other) {
        if (isNegative == other.isNegative) {
            std::string result;
            int carry = 0, i = number.size() - 1, j = other.number.size() - 1;
            while (i >= 0 || j >= 0 || carry) {
                int sum = carry;
                if (i >= 0) sum += number[i--] - '0';
                if (j >= 0) sum += other.number[j--] - '0';
                result.push_back(char('0' + (sum % 10)));
                carry = sum / 10;
            }
            std::reverse(result.begin(), result.end());
            number = result;
        } else {
            *this -= -other;
        }
        removeLeadingZeros();
        return *this;
    }
    friend BigInt operator+(BigInt lhs, const BigInt& rhs) {
        lhs += rhs;
        return lhs;
    }

    // Subtraction
    BigInt& operator-=(const BigInt& other) {
        if (isNegative != other.isNegative) {
            *this += -other;
        } else {
            if (compareMagnitude(other) >= 0) {
                std::string result;
                int borrow = 0, i = number.size() - 1, j = other.number.size() - 1;
                while (i >= 0) {
                    int diff = (number[i] - '0') - borrow;
                    if (j >= 0) diff -= (other.number[j--] - '0');
                    if (diff < 0) {
                        diff += 10;
                        borrow = 1;
                    } else {
                        borrow = 0;
                    }
                    result.push_back(char('0' + diff));
                    i--;
                }
                while (result.size() > 1 && result.back() == '0') result.pop_back();
                std::reverse(result.begin(), result.end());
                number = result;
            } else {
                BigInt temp = other;
                temp -= *this;
                *this = temp;
                isNegative = !isNegative;
            }
        }
        removeLeadingZeros();
        return *this;
    }
    friend BigInt operator-(BigInt lhs, const BigInt& rhs) {
        lhs -= rhs;
        return lhs;
    }

    // Multiplication
    BigInt& operator*=(const BigInt& other) {
        std::string result(number.size() + other.number.size(), '0');
        for (int i = number.size() - 1; i >= 0; --i) {
            int carry = 0;
            for (int j = other.number.size() - 1; j >= 0; --j) {
                int temp = (result[i + j + 1] - '0') +
                           (number[i] - '0') * (other.number[j] - '0') + carry;
                result[i + j + 1] = char('0' + (temp % 10));
                carry = temp / 10;
            }
            result[i] += carry;
        }
        std::size_t startpos = result.find_first_not_of("0");
        if (startpos != std::string::npos)
            number = result.substr(startpos);
        else
            number = "0";
        isNegative = (isNegative != other.isNegative) && number != "0";
        return *this;
    }
    friend BigInt operator*(BigInt lhs, const BigInt& rhs) {
        lhs *= rhs;
        return lhs;
    }

    // Division  <-- MINIMAL FIX: handle signs properly
    BigInt& operator/=(const BigInt& other) {
        if (other.number == "0") throw std::runtime_error("Division by zero");

        // remember final sign
        bool resultNegative = (isNegative != other.isNegative);

        // work with absolute values
        BigInt dividend = *this;
        BigInt divisor = other;
        dividend.isNegative = false;
        divisor.isNegative = false;

        BigInt current("0"), quotient("0");
        for (char c : dividend.number) {
            current = current * BigInt(10) + BigInt(c - '0');
            int count = 0;
            while (current.compareMagnitude(divisor) >= 0) {
                current -= divisor;
                ++count;
            }
            quotient = quotient * BigInt(10) + BigInt(count);
        }

        *this = quotient;
        // apply sign (but zero must not be negative)
        isNegative = (number != "0") && resultNegative;
        removeLeadingZeros();
        return *this;
    }
    friend BigInt operator/(BigInt lhs, const BigInt& rhs) {
        lhs /= rhs;
        return lhs;
    }

    // Modulus  <-- MINIMAL FIX: remainder keeps dividend sign (like C++)
    BigInt& operator%=(const BigInt& other) {
        if (other.number == "0") throw std::runtime_error("Modulo by zero");

        // preserve original dividend sign
        bool dividendNegative = isNegative;

        // compute quotient using operator/ (which now handles sign)
        BigInt quotient = *this / other;

        // remainder = original - quotient * divisor
        BigInt remainder = *this - quotient * other;

        // remainder should have the same sign as the dividend (and zero must be positive)
        remainder.isNegative = (remainder.number != "0") && dividendNegative;
        *this = remainder;
        return *this;
    }
    friend BigInt operator%(BigInt lhs, const BigInt& rhs) {
        lhs %= rhs;
        return lhs;
    }


    // Stream operator
    friend std::ostream& operator<<(std::ostream& os, const BigInt& obj) {
        if (obj.isNegative) os << '-';
        os << obj.number;
        return os;
    }
    // Prefix increment (++a)
    BigInt& operator++() {
        *this += BigInt(1);
        return *this;
    }

    // Postfix increment (a++)
    BigInt operator++(int) {
        BigInt temp = *this;
        *this += BigInt(1);
        return temp;
    }

    // Prefix decrement (--a)
    BigInt& operator--() {
        *this -= BigInt(1);
        return *this;
    }

    // Postfix decrement (a--)
    BigInt operator--(int) {
        BigInt temp = *this;
        *this -= BigInt(1);
        return temp;
    }

};
int main() {
    cout << "=== BigInt Class Test Program ===" << endl << endl;
    cout << "NOTE: All functions are currently empty." << endl;
    cout << "Your task is to implement ALL the functions above." << endl;
    cout << "The tests below will work once you implement them correctly." << endl << endl;


    // Test 1: Constructors and basic output
    cout << "1. Constructors and output:" << endl;
    BigInt a(12345);              // Should create BigInt from integer
    BigInt b("-67890");           // Should create BigInt from string
    BigInt c("0");                // Should handle zero correctly
    BigInt d = a;                 // Should use copy constructor
    cout << "a (from int): " << a << endl;        // Should print "12345"
    cout << "b (from string): " << b << endl;     // Should print "-67890"
    cout << "c (zero): " << c << endl;            // Should print "0"
    cout << "d (copy of a): " << d << endl << endl; // Should print "12345"

    // Test 2: Arithmetic operations
    cout << "2. Arithmetic operations:" << endl;
    cout << "a + b = " << a + b << endl;          // Should calculate 12345 + (-67890)
    cout << "a - b = " << a - b << endl;          // Should calculate 12345 - (-67890)
    cout << "a * b = " << a * b << endl;          // Should calculate 12345 * (-67890)
    cout << "b / a = " << b / a << endl;          // Should calculate (-67890) / 12345
    cout << "a % 100 = " << a % BigInt(100) << endl << endl; // Should calculate 12345 % 100

    // Test 3: Relational operators
    cout << "3. Relational operators:" << endl;
    cout << "a == d: " << (a == d) << endl;       // Should be true (12345 == 12345)
    cout << "a != b: " << (a != b) << endl;       // Should be true (12345 != -67890)
    cout << "a < b: " << (a < b) << endl;         // Should be false (12345 < -67890)
    cout << "a > b: " << (a > b) << endl;         // Should be true (12345 > -67890)
    cout << "c == 0: " << (c == BigInt(0)) << endl << endl; // Should be true (0 == 0)

    // Test 4: Unary operators and increments
    cout << "4. Unary operators and increments:" << endl;
    cout << "-a: " << -a << endl;                 // Should print "-12345"
    cout << "++a: " << ++a << endl;               // Should increment and print "12346"
    cout << "a--: " << a-- << endl;               // Should print "12346" then decrement
    cout << "a after decrement: " << a << endl << endl; // Should print "12345"

    // Test 5: Large number operations
    cout << "5. Large number operations:" << endl;
    BigInt num1("12345678901234567890");
    BigInt num2("98765432109876543210");
    cout << "Very large addition: " << num1 + num2 << endl;
    cout << "Very large multiplication: " << num1 * num2 << endl << endl;

    // Test 6: Edge cases and error handling
    cout << "6. Edge cases:" << endl;
    BigInt zero(0);
    BigInt one(1);
    try {
        BigInt result = one / zero;               // Should throw division by zero error
        cout << "Division by zero succeeded (unexpected)" << endl;
    } catch (const runtime_error& e) {
        cout << "Division by zero correctly threw error: " << e.what() << endl;
    }
    cout << "Multiplication by zero: " << one * zero << endl;        // Should be "0"
    cout << "Negative multiplication: " << BigInt(-5) * BigInt(3) << endl;  // Should be "-15"
    cout << "Negative division: " << BigInt(-10) / BigInt(3) << endl;       // Should be "-3"
    cout << "Negative modulus: " << BigInt(-10) % BigInt(3) << endl;        // Should be "-1"


    return 0;
}
