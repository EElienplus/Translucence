#ifndef TRANSLUCENCE_BIGNUM_HPP
#define TRANSLUCENCE_BIGNUM_HPP

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <sstream>
#include <thread>
#include <map>
#include <mutex>
#include <future>

namespace Math {

template<typename T>
class UBigNumBase {
public:
    std::vector<T> chunks;

    UBigNumBase() {}
    UBigNumBase(unsigned long long v) {
        if (v == 0) {
            chunks.push_back(0);
        } else {
            chunks.push_back((T)v);
            if constexpr (sizeof(unsigned long long) > sizeof(T)) {
                unsigned long long temp = v >> (sizeof(T) * 8);
                while (temp > 0) {
                    chunks.push_back((T)temp);
                    temp >>= (sizeof(T) * 8);
                }
            }
        }
    }

    void operator+=(const UBigNumBase& other) {
        if (other.chunks.size() > chunks.size()) {
            chunks.resize(other.chunks.size(), 0);
        }

        uint64_t carry = 0;
        size_t i = 0;
        size_t other_size = other.chunks.size();

        for (; i < other_size; ++i) {
            unsigned __int128 sum = (unsigned __int128)chunks[i] + other.chunks[i] + carry;
            chunks[i] = (T)sum;
            carry = (uint64_t)(sum >> 64);
        }

        for (; carry && i < chunks.size(); ++i) {
            unsigned __int128 sum = (unsigned __int128)chunks[i] + carry;
            chunks[i] = (T)sum;
            carry = (uint64_t)(sum >> 64);
        }

        if (carry) {
            chunks.push_back((T)carry);
        }
    }

    void operator-=(const UBigNumBase& other) {
        uint64_t borrow = 0;
        size_t i = 0;
        size_t other_size = other.chunks.size();
        for (; i < other_size; ++i) {
            unsigned __int128 diff = (unsigned __int128)chunks[i] - other.chunks[i] - borrow;
            chunks[i] = (T)diff;
            borrow = (uint64_t)(diff >> 127);
        }
        for (; borrow && i < chunks.size(); ++i) {
            unsigned __int128 diff = (unsigned __int128)chunks[i] - borrow;
            chunks[i] = (T)diff;
            borrow = (uint64_t)(diff >> 127);
        }
        normalize();
    }

    bool operator<(const UBigNumBase& other) const {
        if (chunks.size() != other.chunks.size()) {
            return chunks.size() < other.chunks.size();
        }
        for (int i = (int)chunks.size() - 1; i >= 0; --i) {
            if (chunks[i] != other.chunks[i]) {
                return chunks[i] < other.chunks[i];
            }
        }
        return false;
    }

    bool operator>(const UBigNumBase& other) const { return other < *this; }
    bool operator<=(const UBigNumBase& other) const { return !(*this > other); }
    bool operator>=(const UBigNumBase& other) const { return !(*this < other); }
    bool operator!=(const UBigNumBase& other) const { return !(*this == other); }

    bool operator==(const UBigNumBase& other) const {
        return chunks == other.chunks;
    }

    void multiplyBy(uint64_t v) {
        if (v == 0) {
            chunks.assign(1, 0);
            return;
        }
        if (v == 1) return;
        unsigned __int128 carry = 0;
        for (size_t i = 0; i < chunks.size(); ++i) {
            unsigned __int128 res = (unsigned __int128)chunks[i] * v + carry;
            chunks[i] = (T)res;
            carry = res >> (sizeof(T) * 8);
        }
        while (carry > 0) {
            chunks.push_back((T)carry);
            carry >>= (sizeof(T) * 8);
        }
    }

    void multiplyBy10(int power) {
        for (int i = 0; i < power; ++i) {
            multiplyBy(10);
        }
    }

    void operator*=(const UBigNumBase& other) {
        if ((chunks.size() == 1 && chunks[0] == 0) || (other.chunks.size() == 1 && other.chunks[0] == 0)) {
            chunks.assign(1, 0);
            return;
        }

        if (chunks.size() < 64 || other.chunks.size() < 64) {
            multiplySequential(other);
            return;
        }

        *this = multiplyKaratsuba(*this, other);
    }

    static UBigNumBase multiplyKaratsuba(const UBigNumBase& a, const UBigNumBase& b) {
        size_t n = std::max(a.chunks.size(), b.chunks.size());
        if (n < 64) {
            UBigNumBase res = a;
            res.multiplySequential(b);
            return res;
        }

        size_t m = (n + 1) / 2;

        UBigNumBase high1, low1, high2, low2;
        
        if (a.chunks.size() > m) {
            low1.chunks.assign(a.chunks.begin(), a.chunks.begin() + m);
            high1.chunks.assign(a.chunks.begin() + m, a.chunks.end());
        } else {
            low1 = a;
            high1.chunks.assign(1, 0);
        }

        if (b.chunks.size() > m) {
            low2.chunks.assign(b.chunks.begin(), b.chunks.begin() + m);
            high2.chunks.assign(b.chunks.begin() + m, b.chunks.end());
        } else {
            low2 = b;
            high2.chunks.assign(1, 0);
        }
        
        low1.normalize(); high1.normalize();
        low2.normalize(); high2.normalize();

        UBigNumBase z0, z1, z2;
        if (n > 500) {
            std::future<UBigNumBase> z0_fut = std::async(std::launch::async, [&]() { return multiplyKaratsuba(low1, low2); });
            std::future<UBigNumBase> z2_fut = std::async(std::launch::async, [&]() { return multiplyKaratsuba(high1, high2); });
            
            UBigNumBase s1 = low1 + high1;
            UBigNumBase s2 = low2 + high2;
            z1 = multiplyKaratsuba(s1, s2);

            z0 = z0_fut.get();
            z2 = z2_fut.get();
        } else {
            z0 = multiplyKaratsuba(low1, low2);
            z2 = multiplyKaratsuba(high1, high2);
            UBigNumBase s1 = low1 + high1;
            UBigNumBase s2 = low2 + high2;
            z1 = multiplyKaratsuba(s1, s2);
        }

        z1 -= z2;
        z1 -= z0;

        UBigNumBase result = z2;
        result.shiftLeftChunks(2 * m);
        
        z1.shiftLeftChunks(m);
        result += z1;
        result += z0;

        result.normalize();
        return result;
    }

    void shiftLeftChunks(size_t n) {
        if (n == 0) return;
        if (chunks.size() == 1 && chunks[0] == 0) return;
        chunks.insert(chunks.begin(), n, 0);
    }

    void normalize() {
        while (chunks.size() > 1 && chunks.back() == 0) {
            chunks.pop_back();
        }
        if (chunks.empty()) chunks.push_back(0);
    }

    void multiplySequential(const UBigNumBase& other) {
        if (other.chunks.size() == 1) {
            multiplyBy(other.chunks[0]);
            return;
        }
        std::vector<T> result(chunks.size() + other.chunks.size(), 0);
        for (size_t i = 0; i < chunks.size(); ++i) {
            T c_i = chunks[i];
            if (c_i == 0) continue;
            unsigned __int128 carry = 0;
            for (size_t j = 0; j < other.chunks.size(); ++j) {
                unsigned __int128 cur = (unsigned __int128)result[i + j] + (unsigned __int128)c_i * other.chunks[j] + carry;
                result[i + j] = (T)cur;
                carry = cur >> 64;
            }
            if (carry) {
                result[i + other.chunks.size()] += (T)carry;
            }
        }
        chunks = std::move(result);
        normalize();
    }

    void divmod(const UBigNumBase& divisor, UBigNumBase& quotient, UBigNumBase& remainder) const {
        if (divisor.chunks.size() == 1 && divisor.chunks[0] == 0) {
            throw std::runtime_error("Division by zero");
        }
        if (*this < divisor) {
            quotient = UBigNumBase(0);
            remainder = *this;
            return;
        }
        if (divisor.chunks.size() == 1) {
            quotient = *this;
            uint64_t rem = quotient.divideBy(divisor.chunks[0]);
            remainder = UBigNumBase(rem);
            return;
        }

        quotient.chunks.clear();
        remainder.chunks.clear();

        remainder.chunks.assign(1, 0);
        quotient.chunks.resize(chunks.size(), 0);

        for (int i = (int)chunks.size() - 1; i >= 0; --i) {
            T currentChunk = chunks[i];
            for (int bit = 63; bit >= 0; --bit) {
                remainder.shiftLeftOne();
                if ((currentChunk >> bit) & 1) {
                    remainder.chunks[0] |= 1;
                }
                if (remainder >= divisor) {
                    remainder -= divisor;
                    quotient.chunks[i] |= ((uint64_t)1 << bit);
                }
            }
        }
        quotient.normalize();
        remainder.normalize();
    }

    uint64_t divideBy(uint64_t v) {
        if (v == 0) throw std::runtime_error("Division by zero");
        unsigned __int128 rem = 0;
        for (int i = (int)chunks.size() - 1; i >= 0; --i) {
            unsigned __int128 cur = chunks[i] + (rem << (sizeof(T) * 8));
            chunks[i] = (T)(cur / v);
            rem = cur % v;
        }
        while (chunks.size() > 1 && chunks.back() == 0) {
            chunks.pop_back();
        }
        return (uint64_t)rem;
    }

    void shiftLeftOne() {
        uint64_t carry = 0;
        for (size_t i = 0; i < chunks.size(); ++i) {
            uint64_t next_carry = chunks[i] >> 63;
            chunks[i] = (chunks[i] << 1) | carry;
            carry = next_carry;
        }
        if (carry) chunks.push_back(1);
    }

    void operator/=(const UBigNumBase& other) {
        UBigNumBase q, r;
        divmod(other, q, r);
        *this = std::move(q);
    }

    void operator%=(const UBigNumBase& other) {
        UBigNumBase q, r;
        divmod(other, q, r);
        *this = std::move(r);
    }

    UBigNumBase operator/(const UBigNumBase& other) const {
        UBigNumBase q, r;
        divmod(other, q, r);
        return q;
    }

    UBigNumBase operator%(const UBigNumBase& other) const {
        UBigNumBase q, r;
        divmod(other, q, r);
        return r;
    }

    UBigNumBase operator*(const UBigNumBase& other) const {
        UBigNumBase res = *this;
        res *= other;
        return res;
    }

    UBigNumBase operator-(const UBigNumBase& other) const {
        UBigNumBase res = *this;
        res -= other;
        return res;
    }

    UBigNumBase& operator++() {
        *this += UBigNumBase(1);
        return *this;
    }

    UBigNumBase operator++(int) {
        UBigNumBase temp = *this;
        ++(*this);
        return temp;
    }

    UBigNumBase& operator--() {
        *this -= UBigNumBase(1);
        return *this;
    }

    UBigNumBase operator--(int) {
        UBigNumBase temp = *this;
        --(*this);
        return temp;
    }

    UBigNumBase operator+(const UBigNumBase& other) const {
        UBigNumBase res = *this;
        res += other;
        return res;
    }

    static UBigNumBase getPowerOf10(int64_t n) {
        static std::map<int64_t, UBigNumBase> cache;
        static std::mutex mtx;
        {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = cache.find(n);
            if (it != cache.end()) return it->second;
        }

        UBigNumBase res;
        if (n == 0) res = 1;
        else if (n == 1) res = 10;
        else {
            UBigNumBase half = getPowerOf10(n / 2);
            res = half * half;
            if (n % 2 == 1) res.multiplyBy(10);
        }

        std::lock_guard<std::mutex> lock(mtx);
        cache[n] = res;
        return res;
    }

    std::string toStringRecursive() const {
        if (chunks.size() < 128) {
            return toStringSmall();
        }

        static constexpr double log10_2 = 0.301029995663981195;
        double digits = chunks.size() * 64 * log10_2;
        int64_t mid = (int64_t)(digits / 2);
        
        UBigNumBase p = getPowerOf10(mid);
        UBigNumBase q, r;
        divmod(p, q, r);

        std::string s_low, s_high;
        if (chunks.size() > 1024) {
            auto fut = std::async(std::launch::async, [q]() { return q.toStringRecursive(); });
            s_low = r.toStringRecursive();
            s_high = fut.get();
        } else {
            s_high = q.toStringRecursive();
            s_low = r.toStringRecursive();
        }

        if (s_high == "0") return s_low;
        
        std::string res = s_high;
        if ((int64_t)s_low.length() < mid) {
            res.append(mid - s_low.length(), '0');
        }
        res.append(s_low);
        return res;
    }

    std::string toStringSmall() const {
        if (chunks.empty() || (chunks.size() == 1 && chunks[0] == 0)) return "0";

        static constexpr uint64_t DEC_BASE = 1000000000000000000ULL;
        std::vector<uint64_t> decimal_chunks;
        decimal_chunks.reserve(chunks.size() * 20 / 18 + 1);
        decimal_chunks.push_back(0);

        for (int i = (int)chunks.size() - 1; i >= 0; --i) {
            unsigned __int128 carry = chunks[i];
            size_t d_size = decimal_chunks.size();
            for (size_t j = 0; j < d_size; ++j) {
                unsigned __int128 cur = ((unsigned __int128)decimal_chunks[j] << 64) + carry;
                decimal_chunks[j] = (uint64_t)(cur % DEC_BASE);
                carry = cur / DEC_BASE;
            }
            while (carry) {
                decimal_chunks.push_back((uint64_t)(carry % DEC_BASE));
                carry /= DEC_BASE;
            }
        }

        std::string res;
        res.reserve(decimal_chunks.size() * 18 + 1);
        res = std::to_string(decimal_chunks.back());
        for (int i = (int)decimal_chunks.size() - 2; i >= 0; --i) {
            uint64_t v = decimal_chunks[i];
            char buf[19];
            for (int k = 17; k >= 0; --k) {
                buf[k] = (v % 10) + '0';
                v /= 10;
            }
            buf[18] = '\0';
            res.append(buf, 18);
        }
        return res;
    }

    std::string toString() const {
        if (chunks.empty() || (chunks.size() == 1 && chunks[0] == 0)) return "0";
        return toStringRecursive();
    }

    friend std::ostream& operator<<(std::ostream& os, const UBigNumBase& ubn) {
        os << ubn.toString();
        return os;
    }

    friend std::istream& operator>>(std::istream& is, UBigNumBase& ubn) {
        std::string s;
        if (is >> s) {
            ubn.chunks.assign(1, 0);
            for (char c : s) {
                if (isdigit(c)) {
                    ubn.multiplyBy(10);
                    ubn += UBigNumBase((unsigned long long)(c - '0'));
                }
            }
        }
        return is;
    }
};

using UBigNum = UBigNumBase<uint64_t>;

class BigNum {
public:
    bool negative = false;
    UBigNum magnitude;

    BigNum() {}
    BigNum(long long v) {
        if (v < 0) {
            negative = true;
            magnitude = UBigNum((unsigned long long)-v);
        } else {
            magnitude = UBigNum((unsigned long long)v);
        }
    }
    BigNum(const std::string& s) {
        std::stringstream ss(s);
        ss >> *this;
    }

    void operator+=(const BigNum& other) {
        if (negative == other.negative) {
            magnitude += other.magnitude;
        } else {
            if (magnitude < other.magnitude) {
                UBigNum temp = other.magnitude;
                temp -= magnitude;
                magnitude = temp;
                negative = other.negative;
            } else {
                magnitude -= other.magnitude;
                if (magnitude.chunks.size() == 1 && magnitude.chunks[0] == 0) {
                    negative = false;
                }
            }
        }
    }

    void operator-=(const BigNum& other) {
        if (negative != other.negative) {
            magnitude += other.magnitude;
        } else {
            if (magnitude < other.magnitude) {
                UBigNum temp = other.magnitude;
                temp -= magnitude;
                magnitude = temp;
                negative = !negative;
            } else {
                magnitude -= other.magnitude;
                if (magnitude.chunks.size() == 1 && magnitude.chunks[0] == 0) {
                    negative = false;
                }
            }
        }
    }

    BigNum operator-() const {
        BigNum res = *this;
        if (res.magnitude.chunks.size() > 1 || res.magnitude.chunks[0] != 0) {
            res.negative = !res.negative;
        }
        return res;
    }

    BigNum operator-(const BigNum& other) const {
        BigNum res = *this;
        res -= other;
        return res;
    }

    BigNum operator+(const BigNum& other) const {
        BigNum res = *this;
        res += other;
        return res;
    }

    void operator/=(const BigNum& other) {
        negative = (negative != other.negative);
        magnitude /= other.magnitude;
        if (magnitude.chunks.size() == 1 && magnitude.chunks[0] == 0) {
            negative = false;
        }
    }

    BigNum operator/(const BigNum& other) const {
        BigNum res = *this;
        res /= other;
        return res;
    }

    void operator%=(const BigNum& other) {
        // Sign of result matches sign of dividend in C++
        magnitude %= other.magnitude;
        if (magnitude.chunks.size() == 1 && magnitude.chunks[0] == 0) {
            negative = false;
        }
    }

    BigNum operator%(const BigNum& other) const {
        BigNum res = *this;
        res %= other;
        return res;
    }

    void operator*=(const BigNum& other) {
        negative = (negative != other.negative);
        magnitude *= other.magnitude;
        if (magnitude.chunks.size() == 1 && magnitude.chunks[0] == 0) {
            negative = false;
        }
    }

    BigNum operator*(const BigNum& other) const {
        BigNum res = *this;
        res *= other;
        return res;
    }

    bool operator<(const BigNum& other) const {
        if (negative != other.negative) return negative;
        if (negative) return other.magnitude < magnitude;
        return magnitude < other.magnitude;
    }

    bool operator>(const BigNum& other) const { return other < *this; }
    bool operator<=(const BigNum& other) const { return !(*this > other); }
    bool operator>=(const BigNum& other) const { return !(*this < other); }
    bool operator==(const BigNum& other) const {
        return negative == other.negative && magnitude == other.magnitude;
    }
    bool operator!=(const BigNum& other) const { return !(*this == other); }

    BigNum& operator++() {
        *this += BigNum(1);
        return *this;
    }

    BigNum operator++(int) {
        BigNum temp = *this;
        ++(*this);
        return temp;
    }

    BigNum& operator--() {
        *this -= BigNum(1);
        return *this;
    }

    BigNum operator--(int) {
        BigNum temp = *this;
        --(*this);
        return temp;
    }

    std::string toString() const {
        if (magnitude.chunks.empty() || (magnitude.chunks.size() == 1 && magnitude.chunks[0] == 0)) return "0";
        return (negative ? "-" : "") + magnitude.toString();
    }

    friend std::ostream& operator<<(std::ostream& os, const BigNum& bn) {
        os << bn.toString();
        return os;
    }

    friend std::istream& operator>>(std::istream& is, BigNum& bn) {
        std::string s;
        if (is >> s) {
            bool neg = false;
            size_t start = 0;
            if (s[0] == '-') {
                neg = true;
                start = 1;
            }
            bn.magnitude.chunks.assign(1, 0);
            for (size_t i = start; i < s.length(); ++i) {
                if (isdigit(s[i])) {
                    bn.magnitude.multiplyBy(10);
                    bn.magnitude += UBigNum((unsigned long long)(s[i] - '0'));
                }
            }
            bn.negative = neg;
        }
        return is;
    }
};

class BigDecimal {
public:
    BigNum significand;
    int64_t scale = 0; // significand * 10^-scale

    BigDecimal() {}
    BigDecimal(double v) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(30) << v;
        std::string s = ss.str();

        // Remove trailing zeros
        while (s.back() == '0' && s.find('.') != std::string::npos) {
            s.pop_back();
        }
        if (s.back() == '.') s.pop_back();

        bool neg = false;
        if (s[0] == '-') {
            neg = true;
            s.erase(0, 1);
        }

        size_t dot = s.find('.');
        if (dot != std::string::npos) {
            scale = s.length() - dot - 1;
            s.erase(dot, 1);
        }

        // Convert large string to BigNum magnitude
        magnitudeFromString(s);
        significand.negative = neg;
    }

    void magnitudeFromString(const std::string& s) {
        significand.magnitude.chunks.assign(1, 0);
        for (char c : s) {
            significand.magnitude.multiplyBy(10);
            significand.magnitude += UBigNum((unsigned long long)(c - '0'));
        }
    }

    BigDecimal(BigNum sig, int64_t sc) : significand(sig), scale(sc) {
        normalize();
    }

    void normalize() {
        // Could remove trailing zeros from significand and decrease scale
    }

    void align(BigDecimal& other) {
        if (scale < other.scale) {
            significand.magnitude.multiplyBy10((int)(other.scale - scale));
            scale = other.scale;
        } else if (other.scale < scale) {
            other.significand.magnitude.multiplyBy10((int)(scale - other.scale));
            other.scale = scale;
        }
    }

    void operator+=(BigDecimal other) {
        align(other);
        significand += other.significand;
    }

    BigDecimal operator+(const BigDecimal& other) const {
        BigDecimal res = *this;
        res += other;
        return res;
    }

    void operator-=(BigDecimal other) {
        align(other);
        significand -= other.significand;
    }

    BigDecimal operator-(const BigDecimal& other) const {
        BigDecimal res = *this;
        res -= other;
        return res;
    }

    void operator*=(BigDecimal other) {
        significand *= other.significand;
        scale += other.scale;
    }

    BigDecimal operator*(const BigDecimal& other) const {
        BigDecimal res = *this;
        res *= other;
        return res;
    }

    void operator/=(BigDecimal other) {
        if (other.significand == BigNum(0)) {
            throw std::runtime_error("Division by zero");
        }
        // To maintain precision, we can scale up the dividend
        int precision = 30;
        significand.magnitude.multiplyBy10(precision);
        scale += precision; // Should BE scale += precision because we made SIGNIFICAND larger, so to keep value same, scale must increase?
        // No, significand * 10^-scale. If significand * 10^30, then scale should increase by 30.
        // Value = (significand * 10^30) * 10^-(scale+30) = significand * 10^-scale. Correct.

        significand /= other.significand;
        scale -= other.scale;

        // Final scale = (original_scale + precision) - other_scale
    }

    BigDecimal operator/(const BigDecimal& other) const {
        BigDecimal res = *this;
        res /= other;
        return res;
    }

    bool operator<(const BigDecimal& other) const {
        BigDecimal a = *this;
        BigDecimal b = other;
        a.align(b);
        return a.significand < b.significand;
    }

    bool operator>(const BigDecimal& other) const { return other < *this; }
    bool operator<=(const BigDecimal& other) const { return !(*this > other); }
    bool operator>=(const BigDecimal& other) const { return !(*this < other); }
    bool operator==(const BigDecimal& other) const {
        BigDecimal a = *this;
        BigDecimal b = other;
        a.align(b);
        return a.significand == b.significand;
    }
    bool operator!=(const BigDecimal& other) const { return !(*this == other); }

    std::string toString() const {
        std::cout << "\r[BigDecimal] Stitching numbers..." << std::flush;
        std::string s = significand.toString();
        bool neg = false;
        if (s[0] == '-') {
            neg = true;
            s.erase(0, 1);
        }

        std::string res;
        res.reserve(s.length() + (scale > 0 ? scale + 1 : 0) + 1);
        if (scale <= 0) {
            res = s;
            res.append(-scale, '0');
        } else if ((int64_t)s.length() <= scale) {
            res = "0.";
            res.append(scale - s.length(), '0');
            res.append(s);
        } else {
            res = s;
            res.insert(s.length() - scale, ".");
        }

        // Remove trailing zeros after decimal point
        if (res.find('.') != std::string::npos) {
            while (res.back() == '0') res.pop_back();
            if (res.back() == '.') res.pop_back();
        }

        if (res == "0" || res == "") return "0";
        return (neg ? "-" : "") + res;
    }

    friend std::ostream& operator<<(std::ostream& os, const BigDecimal& bd) {
        os << bd.toString();
        return os;
    }

    friend std::istream& operator>>(std::istream& is, BigDecimal& bd) {
        std::string s;
        if (is >> s) {
            bd = BigDecimal(0.0);
            bool neg = false;
            size_t start = 0;
            if (s[0] == '-') {
                neg = true;
                start = 1;
            }
            size_t dot = s.find('.');
            if (dot != std::string::npos) {
                bd.scale = s.length() - dot - 1;
                s.erase(dot, 1);
            } else {
                bd.scale = 0;
            }
            bd.magnitudeFromString(s.substr(start));
            bd.significand.negative = neg;
        }
        return is;
    }
};

} // namespace Math

#endif // TRANSLUCENCE_BIGNUM_HPP
