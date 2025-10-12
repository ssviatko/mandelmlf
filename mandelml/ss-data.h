#ifndef SS_DATA_H
#define SS_DATA_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <exception>
#include <deque>
#include <vector>
#include <map>
#include <stack>
#include <algorithm>

#include <memory.h>
#include <stdint.h>
#include <netinet/in.h>

#include "bf.h"
#include "md5.h"
#include "sha1.h"
#include "sha2.h"

// 64-bit endian swap macros
#if defined(__linux__)

#include <bits/byteswap.h>

#define htobe64(x) __bswap_64 (x)
#define htole64(x) (x)
#define be64toh(x) __bswap_64 (x)
#define le64toh(x) (x)

#elif defined(__APPLE__)

#include <libkern/OSByteOrder.h>

#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

#define __BYTE_ORDER    BYTE_ORDER
#define __BIG_ENDIAN    BIG_ENDIAN
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#define __PDP_ENDIAN    PDP_ENDIAN

#endif

namespace ss {

/**
 * @brief Main exception class for data errors.
 **/

class data_exception : public std::exception {
public:
	/**
	 * @brief Construct exception object (C string).
	 *
	 * @param a_what The error message.
	 *
	 * Construct an exception object using a C string as the source for the error message.
	 **/

	data_exception(const char* a_what) throw();

	/**
	 * @brief Construct exception object (std::string).
	 *
	 * @param a_what The error message.
	 *
	 * Construct an exception object using a std::string as the source for the error message.
	 **/

	data_exception(const std::string& a_what) throw();
	~data_exception() throw() { } ///< Destroy exception object.

	/**
	 * @brief Return error string.
	 *
	 * Return the contents of the "what" string contained in the exception object.
	 **/

	virtual const char* what() const throw();
	
protected:
	std::string m_what; ///< Error string.
};

/**
 * @brief Data class.
 *
 * The data class abstracts a data buffer of arbitrary size which can expand or
 * contract as needed. It has routines for circular reads/writes, loading and saving
 * to files, operations with delimiters, awareness of endianness, Pascal strings,
 * and other features to make use of a data buffer convenient in C++.
 **/

class data {
	/* private constants */
	const static int debug = 0; ///< Compile with this constant set non-zero to receive debug messages.
	const static uint32_t crc32_tab[];
	const static uint8_t byte_mask[];

	/* huffman related */

	class huff_tree_node {
	public:
		enum { LEAF, INTERNAL };
		huff_tree_node() : type(LEAF), id(0), symbol(0), freq(0), left_id(-1), right_id(-1) { }
		int16_t type;
		int16_t id;
		uint8_t symbol;
		uint64_t freq;
		int16_t left_id;
		int16_t right_id;
		bool operator<(const huff_tree_node& rhs) const { return (freq < rhs.freq); }
	};

	const static uint32_t HUFF_MAGIC_COOKIE = 0xc0edbabe;

	/* support routines */

	/**
	 * @brief Construct data buffer with contents of another data object
	 **/

	void copy_construct(const data& a_data);

	/**
	 * @brief Allocate space for a new buffer, and optionally copy the old one.
	 *
	 * @param a_capacity Size in bytes to allocate
	 * @param a_copyold Flag whether to move the old data forward into the new buffer
	 *
	 * The core of memory management for the data class. This method is called
	 * internally whenever the buffer needs to be allocated or reallocated. An
	 * optional parameter flags whether the contents of the old buffer are to
	 * be copied. If this flag is false, then the buffer is allocated and both
	 * cursors and the delimiter are reset to their default values.
	 **/

	void allocate_buffer(uint64_t a_capacity, bool a_copyold);

	/**
	 * @brief Truncate the front of the buffer - Circular buffer support routine
	 *
	 * @param a_new_front Byte position where the new start of buffer should be
	 *
	 * This method moves the entire contents of the buffer down by a_new_front
	 * bytes. For instance, if the buffer contains 100 bytes and a_new_front is
	 * 15, then byte 15 becomes the new 0 position, everything moves down, and
	 * the length of the buffer will now be 85 bytes. No memory reallocations
	 * take place.
	 *
	 * FUTURE DIRECTIONS:
	 * Replace for loop with a overlap-aware form of memcpy, a machine language
	 * subroutine, or anything else that could improve the speed of this routine.
	 **/

	void truncate_front(uint64_t a_new_front);
	
public:
	/* public constants */
	const static uint64_t DEFAULT_BUFFER_CAPACITY = 4096; ///< Default buffer capacity. This is the size (in bytes) of a buffer when it is initially created.
	const static uint64_t DEFAULT_BUFFER_INCREMENT = 1024; ///< Default buffer increment. This is the number of bytes by which the buffer increments when extra space is needed.

	const static char DELIMITER_NULL = 0; ///< NULL character delimiter.
	const static char DELIMITER_TAB = 9; ///< Tab character.
	const static char DELIMITER_LF = 0x0a; ///< Line Feed character.
	const static char DELIMITER_CR = 0x0d; ///< Carriage Return character.

	class bit_cursor {
	public:
		bit_cursor() : byte(0), bit(7) { }
		// bits start at bit 7 and count down to 0
		uint64_t byte;
		uint16_t bit;
		void set_absolute(uint64_t a_absolute); // lower 64 bits of absolute bit position
		void advance_to_next_whole_byte() { if (bit < 7) { ++byte; bit = 7; } }
		uint64_t get_absolute();
	};

	/* constructors, etc */
	data();
	data(const char *a_str); // construct from null-terminated C string
	data(uint64_t a_capacity); // construct with specified capacity
	data(uint8_t *a_addr, uint64_t a_len); // construct emplaced
	data(const data& a_data); // copy ctor
	~data(); // dtor
	
	void empty(); // destroy and re-create buffer
	void hexdump(uint64_t a_len); // dump hex bytes; advance read cursor.
	void append(const data& a_data); // append another data object

	/* emplacement */
	void emplace(uint8_t *a_addr, uint64_t a_len); // point data object towards existing memory
	bool emplaced() { return m_emplaced; } // returns true if emplaced, false if normally allocated
	void unemplace(); // return to normal allocation

	/* files */
	void save_file(const char *filename); // save entire buffer to file
	void load_file(const char *filename); // load contents of file at write cursor position
	
	/* getters & setters, cursor movement */
	uint64_t get_buffer_capacity() const;
	uint64_t get_buffer_datalen() const;
	uint64_t get_read_cursor() const;
	uint64_t get_write_cursor() const;
	bit_cursor get_read_bit_cursor() const { return m_read_bit_cursor; }
	bit_cursor get_write_bit_cursor() const { return m_write_bit_cursor; }
	void set_read_cursor(uint64_t a_pos);
	void set_write_cursor(uint64_t a_pos);
	void set_write_cursor_to_append(); // write cursor to m_buffer_datalen + 1
	void set_read_bit_cursor(bit_cursor a_bit_cursor);
	void set_write_bit_cursor(bit_cursor a_bit_cursor);
	void set_write_bit_cursor_to_append(); // write cursor at bit 7, m_buffer_datalen + 1
	const char *c_str(); // present the buffer as a printable C string
	const char *buffer() { return (const char *)m_buffer; } // expose the buffer itself
	void set_network_byte_order(bool a_order);
	bool get_network_byte_order() const;
	void set_delimiter(char a_delim);
	char get_delimiter() const;

	/* readers & writers */

	/* bits */
	void write_true(); // write 1 bit at bit cursor, value 1 (true)
	void write_false(); // write 1 bit at bit cursor, value 0 (false);
	void write_bit(bool a_bit);
	void write_bit(uint64_t a_bit); // integer version: 0=false, >0 = true, 
	void write_bits(uint64_t a_bits, uint16_t a_count);
	bool read_bit();
	uint64_t read_bits(uint16_t a_count);

	// raw data read/write
	void write_data(uint8_t *a_data, uint64_t a_len);
	void fill(uint8_t a_char, uint64_t a_len);
	void read_data(uint8_t *a_data, uint64_t a_len);

	// deal with C strings
	void write_c_str(const char *a_str); // write C string (no null at end)
	void write_c_str_delim(const char *a_str); // write C string with delimiter char at end
	void write_delim(); // write delimiter character

	bool read_c_str_delim(char *a_str); // read C string up to delimiter char and null-terminate (delimiter not copied)

	// std::strings
	void write_std_str(const std::string& a_str);
	void write_std_str_delim(const std::string& a_str);

	std::string read_std_str(uint64_t a_len);
	bool read_std_str_delim(std::string& a_str);

	// Pascal strings
	std::string read_std_str_pascal16();
	void write_std_str_pascal16(const std::string& a_str);
	
	// integers of all kinds
	void write_uint8(uint8_t a_uint8);
	uint8_t read_uint8();
	void write_int8(int8_t a_int8);
	int8_t read_int8();

	void write_uint16(uint16_t a_uint16);
	uint16_t read_uint16();
	void write_int16(int16_t a_int16);
	int16_t read_int16();

	void write_uint32(uint32_t a_uint32);
	uint32_t read_uint32();
	void write_int32(int32_t a_int32);
	int32_t read_int32();
	
	void write_uint64(uint64_t a_uint64);
	uint64_t read_uint64();
	void write_int64(int64_t a_int64);
	int64_t read_int64();
	
	// floating point numbers
	void write_float(float a_float);
	float read_float();

	void write_double(double a_double);
	double read_double();
	
	/* circular buffer routines */
	void push_back(uint8_t *a_data, uint64_t a_datalen);
	void push_back_c_str(const char *a_str); // push back a C string, no null term or delimiter
	void push_back_c_str_delim(const char *a_str); // same as above, but write delim at end
	void push_back_std_str(const std::string& a_str);
	void push_back_std_str_delim(const std::string& a_str);
	
	uint64_t pop_front(uint8_t *a_data, uint64_t a_datalen);
	uint64_t pop_front_c_str(char *a_str, uint64_t a_datalen);
	bool pop_front_c_str_delim(char *a_str); // pop up until delim character
	uint64_t pop_front_std_str(std::string& a_str, uint64_t a_datalen);
	bool pop_front_std_str_delim(std::string& a_str);

	/* operators */
	uint8_t& operator[](uint64_t index);
	data& operator=(const data& a_data);
	data& operator+=(const data& a_data);

	/* static freely-available utility methods */
	static std::string hex_str(const uint8_t *a_data, int a_len);
	static uint8_t *hex_decode(const std::string a_str, uint32_t *decode_len);
	static std::string base64_str(const uint8_t *a_data, int a_len);
	static uint8_t *base64_decode(const std::string a_str, uint32_t *decode_len);

	/* textual presentation and initialization */
	void write_hex_str(const std::string& a_str);
	std::string read_hex_str(uint64_t a_len);
	void write_base64(const std::string& a_str);
	std::string read_base64(uint64_t a_len);
	void convert_to_base64();
	void convert_from_base64();

	/* hashing */
	uint32_t crc32(uint32_t a_crc);
	data md5();
	data sha1();
	data sha224();
	data sha256();
	data sha384();
	data sha512();

	/* encryption */
	void encrypt_bf_cbc(bf::key& a_key);
	void decrypt_bf_cbc(bf::key& a_key);
	void encrypt_bf_cbc_hmac_sha256(bf::key& a_key);
	void decrypt_bf_cbc_hmac_sha256(bf::key& a_key);

	/* data compression */
	void rle_encode();
	void rle_decode();
	void huffman_encode();
	void huffman_decode();
	
protected:
	uint8_t *m_buffer;
	uint64_t m_buffer_capacity;
	uint64_t m_buffer_datalen;
	uint64_t m_read_cursor;
	uint64_t m_write_cursor;
	bit_cursor m_read_bit_cursor;
	bit_cursor m_write_bit_cursor;
	char m_delim; // delimiter character for delimiter-mode reads/writes
	bool m_network_byte_order; // byte ordering for cursor-mode writes of integers
	bool m_emplaced; // set if we're pointing to fixed memory
};

} // end namespace ss

#endif // SS_DATA_H
