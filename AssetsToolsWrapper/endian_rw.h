#pragma once
#include <iostream>

template<typename T, int bytes = sizeof(T)>
void endians(T& v) {
	auto p = (unsigned char*)(&v);
	for (auto i = 0; i<bytes / 2; i++) {
		std::swap(p[i], p[bytes - i - 1]);
	}
}

template<typename T>
void operator>(std::istream& stream, T& rhs) {
	stream.read((char*)&rhs, sizeof(T));
	endians(rhs);
}

template<>
inline void operator>(std::istream& stream, std::string& str) {
	str.clear();
	char sz;
	stream.get(sz);
	while (sz) {
		str += sz;
		stream.get(sz);
	}
}

template<typename T>
void operator<(std::istream& stream, T& rhs) {
	stream.read((char*)&rhs, sizeof(T));
}

template<>
inline void operator<(std::istream& stream, std::string& str) {
	str.clear();
	char sz;
	stream.get(sz);
	while (sz) {
		str += sz;
		stream.get(sz);
	}
}

template<typename T>
void operator>(std::ostream& stream, const T& rhs_) {
	T rhs = rhs_;
	endians(rhs);
	stream.write((char*)&rhs, sizeof(T));
}

template<>
inline void operator>(std::ostream& stream, const std::string& str) {
	stream.write(str.data(), str.length());
}

template<typename T>
void operator<(std::ostream& stream, const T& rhs) {
	stream.write((char*)&rhs, sizeof(T));
}

template<>
inline void operator<(std::ostream& stream, const std::string& str) {
	stream.write(str.data(), str.length());
}

inline bool copy(std::istream& src, std::ostream& dst, bool reset = true) {
	if (reset) {
		src.seekg(0, std::ios::beg);
		dst.seekp(0, std::ios::beg);
	}
	char buf[1024];
	while (src.eof()) {
		auto size = src.readsome(buf, 1024);
		dst.write(buf, size);
	}
	return true;
}