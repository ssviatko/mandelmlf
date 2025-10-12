#ifndef SS_MISC_H
#define SS_MISC_H

#include <iostream>
#include <iomanip>
#include <array>
#include <functional>
#include <chrono>
#include <sstream>
#include <cstdint>

#include "ss-data.h"

namespace ansi {

class manip {
public:
	manip(const std::string& a_code) : m_code(a_code) { }
	friend std::ostream& operator<< (std::ostream& os, const manip& a_manip);
protected:
	std::string m_code;
};

const manip black("\033[30m");
const manip red("\033[31m");
const manip green("\033[32m");
const manip yellow("\033[33m");
const manip blue("\033[34m");
const manip magenta("\033[35m");
const manip cyan("\033[36m");
const manip lightgray("\033[37m");
const manip darkgray("\033[90m");
const manip lightred("\033[91m");
const manip lightgreen("\033[92m");
const manip lightyellow("\033[93m");
const manip lightblue("\033[94m");
const manip lightmagenta("\033[95m");
const manip lightcyan("\033[96m");
const manip white("\033[97m");
const manip def("\033[39m");

const manip bgblack("\033[40m");
const manip bgred("\033[41m");
const manip bggreen("\033[42m");
const manip bgyellow("\033[43m");
const manip bgblue("\033[44m");
const manip bgmagenta("\033[45m");
const manip bgcyan("\033[46m");
const manip bglightgray("\033[47m");
const manip bgdarkgray("\033[100m");
const manip bglightred("\033[101m");
const manip bglightgreen("\033[102m");
const manip bglightyellow("\033[103m");
const manip bglightblue("\033[104m");
const manip bglightmagenta("\033[105m");
const manip bglightcyan("\033[106m");
const manip bgwhite("\033[107m");
const manip bgdef("\033[49m");

const manip setbold("\033[1m");
const manip setdim("\033[2m");
const manip setunderlined("\033[4m");
const manip setblink("\033[5m");
const manip setreverse("\033[7m");
const manip sethidden("\033[8m");

const manip resetall("\033[0m");
const manip resetbold("\033[21m");
const manip resetdim("\033[22m");
const manip resetunderlined("\033[24m");
const manip resetblink("\033[25m");
const manip resetreverse("\033[27m");
const manip resethidden("\033[28m");

const std::array<manip, 16> color = { black, red, green, yellow, blue, magenta, cyan, lightgray,
	darkgray, lightred, lightgreen, lightyellow, lightblue, lightmagenta, lightcyan, white };
const std::array<manip, 16> bgcolor = { bgblack, bgred, bggreen, bgyellow, bgblue, bgmagenta, bgcyan, bglightgray,
	bgdarkgray, bglightred, bglightgreen, bglightyellow, bglightblue, bglightmagenta, bglightcyan, bgwhite };

} // namespace ansi

namespace ss {

class poststream : public std::ostream {
        class poststream_stringbuf : public std::stringbuf {
        public:
                poststream_stringbuf(std::function<void(const std::string&)> a_cb) : m_cb(a_cb) { }
                ~poststream_stringbuf() { }
                virtual int sync();
                std::function<void(const std::string&)> m_cb = nullptr;
        };
public:
        poststream(std::function<void(const std::string&)> a_cb)
                : std::ios(0), std::ostream(new poststream_stringbuf(a_cb)) { }
        virtual ~poststream() { delete rdbuf(); }
};

std::string iso8601_us();

class targa24 {
public:
	targa24(uint32_t a_width, uint32_t a_height);
	~targa24();
	void plot(uint32_t a_x, uint32_t a_y, uint8_t a_blue, uint8_t a_green, uint8_t a_red);
	void write_image(const std::string& a_filename);
protected:
	uint32_t m_width, m_height;
	data m_data;
};

} // namespace ss

#endif // SS_MISC_H
