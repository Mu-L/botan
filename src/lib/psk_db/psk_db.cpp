/*
* (C) 2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <botan/psk_db.h>

#include <botan/base64.h>
#include <botan/block_cipher.h>
#include <botan/exceptn.h>
#include <botan/mac.h>
#include <botan/nist_keywrap.h>
#include <botan/internal/mem_utils.h>

namespace Botan {

std::string PSK_Database::get_str(std::string_view name) const {
   return bytes_to_string(this->get(name));
}

void PSK_Database::set_str(std::string_view name, std::string_view psk) {
   auto pskb = as_span_of_bytes(psk);
   this->set(name, pskb.data(), pskb.size());
}

Encrypted_PSK_Database::Encrypted_PSK_Database(const secure_vector<uint8_t>& master_key) {
   m_cipher = BlockCipher::create_or_throw("AES-256");
   m_hmac = MessageAuthenticationCode::create_or_throw("HMAC(SHA-256)");
   m_hmac->set_key(master_key);

   m_cipher->set_key(m_hmac->process("wrap"));
   m_hmac->set_key(m_hmac->process("hmac"));
}

Encrypted_PSK_Database::~Encrypted_PSK_Database() = default;

std::set<std::string> Encrypted_PSK_Database::list_names() const {
   const std::set<std::string> encrypted_names = kv_get_all();

   std::set<std::string> names;

   for(const auto& enc_name : encrypted_names) {
      try {
         const secure_vector<uint8_t> raw_name = base64_decode(enc_name);
         const secure_vector<uint8_t> name_bits = nist_key_unwrap_padded(raw_name.data(), raw_name.size(), *m_cipher);

         const auto pt_name = bytes_to_string(name_bits);
         names.insert(pt_name);
      } catch(Invalid_Authentication_Tag&) {}
   }

   return names;
}

void Encrypted_PSK_Database::remove(std::string_view name) {
   const auto wrapped_name = nist_key_wrap_padded(as_span_of_bytes(name), *m_cipher);

   this->kv_del(base64_encode(wrapped_name));
}

secure_vector<uint8_t> Encrypted_PSK_Database::get(std::string_view name) const {
   const auto wrapped_name = nist_key_wrap_padded(as_span_of_bytes(name), *m_cipher);

   const std::string val_base64 = kv_get(base64_encode(wrapped_name));

   if(val_base64.empty()) {
      throw Invalid_Argument("Named PSK not located");
   }

   const secure_vector<uint8_t> val = base64_decode(val_base64);

   auto wrap_cipher = m_cipher->new_object();
   wrap_cipher->set_key(m_hmac->process(wrapped_name));

   return nist_key_unwrap_padded(val.data(), val.size(), *wrap_cipher);
}

void Encrypted_PSK_Database::set(std::string_view name, const uint8_t val[], size_t len) {
   /*
   * Both as a basic precaution wrt key seperation, and specifically to prevent
   * cut-and-paste attacks against the database, each PSK is encrypted with a
   * distinct key which is derived by hashing the wrapped key name with HMAC.
   */
   const auto wrapped_name = nist_key_wrap_padded(as_span_of_bytes(name), *m_cipher);

   auto wrap_cipher = m_cipher->new_object();
   wrap_cipher->set_key(m_hmac->process(wrapped_name));
   const std::vector<uint8_t> wrapped_key = nist_key_wrap_padded(val, len, *wrap_cipher);

   this->kv_set(base64_encode(wrapped_name), base64_encode(wrapped_key));
}

}  // namespace Botan
