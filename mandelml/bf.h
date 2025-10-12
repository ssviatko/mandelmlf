/*
 *  blowfish.h
 *  blow
 *
 *  Created by Stephen Sviatko on 03/11/17.
 *  Copyright 2017 Good Neighbors LLC. All rights reserved.
 *
 */

#ifndef BF_H
#define BF_H

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <exception>

#include <stdint.h>
#include <memory.h>
#include <arpa/inet.h>

#include "sha2.h"
#include "hmac.h"

namespace bf {

typedef union {
	uint8_t inb[8];
	struct {
		uint32_t L;
		uint32_t R;
	};
} block_t;

// hex_str function used by key and block
std::string hex_str(const uint8_t *a_data, int a_len);

class bf_exception : public std::exception {
public:
	bf_exception(const char *a_what) throw() { m_what = a_what; }
	bf_exception(const std::string& a_what) throw() { m_what = a_what; }
	~bf_exception() throw() { }
	virtual const char *what() const throw() { return m_what.c_str(); }
protected:
	std::string m_what;
};

class key {
	uint8_t m_keydata[56];
	int m_keylen;
	uint8_t m_iv[8];
public:
	key(uint8_t *a_buffer, int a_buffer_len);
	void reschedule(uint8_t *a_buffer, int a_buffer_len);
	void set_iv(uint8_t *a_buffer) { memcpy(m_iv, a_buffer, 8); }

	const uint8_t *get_keydata() const { return (const uint8_t *)m_keydata; }
	int get_keylen() const { return m_keylen; }
	const uint8_t *get_ivdata() const { return (const uint8_t *)m_iv; }

	// stream insertion
	friend std::ostream& operator<<(std::ostream& os, const key& a_key);

	// key factories
	static key key_from_passphrase(const std::string& a_phrase);
};

class key7 {
	uint8_t m_keydata_a[56];
	uint8_t m_keydata_b[56];
	uint8_t m_keydata_c[56];
	uint8_t m_keydata_d[56];
	uint8_t m_iv_abcd[8];

	uint8_t m_keydata_e[56];
	uint8_t m_keydata_f[56];
	uint8_t m_iv_ef[8];

	uint8_t m_keydata_g[56];
	uint8_t m_iv_g[8];

	uint8_t m_iv[32]; // 256 bit overall inter-block IV

public:
	key7(uint8_t *a_buffer); // 3328 bit key
	key7(const std::string& a_file); // load from file

	void reschedule(uint8_t *a_buffer);
	void save_file(const std::string& a_file);
	void set_iv(uint8_t *a_buffer) { memcpy(m_iv, a_buffer, 32); }

	// get partial keys
	const uint8_t *get_keydata_a() const { return (const uint8_t *)m_keydata_a; }
	const uint8_t *get_keydata_b() const { return (const uint8_t *)m_keydata_b; }
	const uint8_t *get_keydata_c() const { return (const uint8_t *)m_keydata_c; }
	const uint8_t *get_keydata_d() const { return (const uint8_t *)m_keydata_d; }
	const uint8_t *get_keydata_e() const { return (const uint8_t *)m_keydata_e; }
	const uint8_t *get_keydata_f() const { return (const uint8_t *)m_keydata_f; }
	const uint8_t *get_keydata_g() const { return (const uint8_t *)m_keydata_g; }

	const uint8_t *get_ivdata() const { return (const uint8_t *)m_iv; } // return 256 bit IV

	// get partial IVs
	const uint8_t *get_ivdata_abcd() const { return (const uint8_t *)m_iv_abcd; }
	const uint8_t *get_ivdata_ef() const { return (const uint8_t *)m_iv_ef; }
	const uint8_t *get_ivdata_g() const { return (const uint8_t *)m_iv_g; }

	// key factories
	static key7 key7_from_passphrase(const std::string& a_phrase);
};

class block {
	// original contents of P/S boxes
	static const uint32_t ORIG_P[16 + 2];
	static const uint32_t ORIG_S[4][256];

	// P/S boxes
	uint32_t P[16 + 2];
	uint32_t S[4][256];

	// 16 rounds is standard
	static const int ROUNDS = 16;

	// private internal routines
	uint32_t F(uint32_t x);
	void Blowfish_Encrypt(uint32_t *xl, uint32_t *xr);
	void Blowfish_Decrypt(uint32_t *xl, uint32_t *xr);
	void Blowfish_Init(const uint8_t *key, int keyLen);

	// IV from key instance
	uint8_t m_iv[8];
public:
	block(const uint8_t *a_buffer, key& a_key);
	void reschedule(const uint8_t *a_buffer, key& a_key);
	void set_blockdata(const uint8_t *a_buffer);
	const uint8_t *get_blockdata() const { return (const uint8_t *)m_block.inb; }

	void encrypt();
	void decrypt();

	void encrypt_with_cbc();
	void decrypt_with_cbc();

	// stream insertion
	friend std::ostream& operator<<(std::ostream& os, const block& a_block);

protected:
	block_t m_block;
};

class bf_cbc {
public:
	bf_cbc(const uint8_t *a_buffer, int a_buffer_len, key& a_key);
	virtual ~bf_cbc();
	void reschedule(const uint8_t *a_buffer, int a_buffer_len, key& a_key);
	const uint8_t *get_output_buffer() const { return (const uint8_t *)m_output_buffer; }
	const int get_output_buffer_len() const { return m_output_buffer_len; }
	virtual void encrypt();
	virtual void decrypt();

protected:
	const uint8_t *m_input_buffer;
	int m_input_buffer_len;
	uint8_t *m_output_buffer;
	int m_output_buffer_len;
	bool m_allocated;
	block m_work;
};

class bf_cbc_hmac_sha256 : public bf_cbc {
public:
	bf_cbc_hmac_sha256(const uint8_t *a_buffer, int a_buffer_len, key& a_key);
	virtual ~bf_cbc_hmac_sha256() { }
	virtual void encrypt();
	virtual void decrypt();
protected:
	// after we schedule the key, we need to hang on to it for hmac computations
	uint8_t m_keydata_save[56];
	int m_keylen_save;
};

} // namespace bf

#endif // BF_H

