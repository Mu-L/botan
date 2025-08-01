/*
* Symmetric Algorithm Base Class
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SYMMETRIC_ALGORITHM_H_
#define BOTAN_SYMMETRIC_ALGORITHM_H_

#include <botan/types.h>

#include <span>
#include <string>

namespace Botan {

class OctetString;

/**
* Represents the length requirements on an algorithm key
*/
class BOTAN_PUBLIC_API(2, 0) Key_Length_Specification final {
   public:
      /**
      * Constructor for fixed length keys
      * @param keylen the supported key length
      */
      explicit Key_Length_Specification(size_t keylen) : m_min_keylen(keylen), m_max_keylen(keylen), m_keylen_mod(1) {}

      /**
      * Constructor for variable length keys
      * @param min_k the smallest supported key length
      * @param max_k the largest supported key length
      * @param k_mod the number of bytes the key must be a multiple of
      */
      Key_Length_Specification(size_t min_k, size_t max_k, size_t k_mod = 1) :
            m_min_keylen(min_k), m_max_keylen(max_k > 0 ? max_k : min_k), m_keylen_mod(k_mod) {}

      /**
      * @param length is a key length in bytes
      * @return true iff this length is a valid length for this algo
      */
      bool valid_keylength(size_t length) const {
         return ((length >= m_min_keylen) && (length <= m_max_keylen) && (length % m_keylen_mod == 0));
      }

      /**
      * @return minimum key length in bytes
      */
      size_t minimum_keylength() const { return m_min_keylen; }

      /**
      * @return maximum key length in bytes
      */
      size_t maximum_keylength() const { return m_max_keylen; }

      /**
      * @return key length multiple in bytes
      */
      size_t keylength_multiple() const { return m_keylen_mod; }

      /*
      * Multiplies all length requirements with the given factor
      * @param n the multiplication factor
      * @return a key length specification multiplied by the factor
      */
      Key_Length_Specification multiple(size_t n) const {
         return Key_Length_Specification(n * m_min_keylen, n * m_max_keylen, n * m_keylen_mod);
      }

   private:
      size_t m_min_keylen, m_max_keylen, m_keylen_mod;
};

/**
* This class represents a symmetric algorithm object.
*/
class BOTAN_PUBLIC_API(2, 0) SymmetricAlgorithm {
   public:
      SymmetricAlgorithm() = default;
      virtual ~SymmetricAlgorithm() = default;
      SymmetricAlgorithm(const SymmetricAlgorithm& other) = default;
      SymmetricAlgorithm(SymmetricAlgorithm&& other) = default;
      SymmetricAlgorithm& operator=(const SymmetricAlgorithm& other) = default;
      SymmetricAlgorithm& operator=(SymmetricAlgorithm&& other) = default;

      /**
      * Reset the internal state. This includes not just the key, but
      * any partial message that may have been in process.
      */
      virtual void clear() = 0;

      /**
      * @return object describing limits on key size
      */
      virtual Key_Length_Specification key_spec() const = 0;

      /**
      * @return maximum allowed key length
      */
      size_t maximum_keylength() const { return key_spec().maximum_keylength(); }

      /**
      * @return minimum allowed key length
      */
      size_t minimum_keylength() const { return key_spec().minimum_keylength(); }

      /**
      * Check whether a given key length is valid for this algorithm.
      * @param length the key length to be checked.
      * @return true if the key length is valid.
      */
      bool valid_keylength(size_t length) const { return key_spec().valid_keylength(length); }

      /**
      * Set the symmetric key of this object.
      * @param key the SymmetricKey to be set.
      */
      void set_key(const OctetString& key);

      /**
      * Set the symmetric key of this object.
      * @param key the contiguous byte range to be set.
      */
      void set_key(std::span<const uint8_t> key);

      /**
      * Set the symmetric key of this object.
      * @param key the to be set as a byte array.
      * @param length in bytes of key param
      */
      void set_key(const uint8_t key[], size_t length) { set_key(std::span{key, length}); }

      /**
      * @return the algorithm name
      */
      virtual std::string name() const = 0;

      /**
      * @return true if a key has been set on this object
      */
      virtual bool has_keying_material() const = 0;

   protected:
      void assert_key_material_set() const { assert_key_material_set(has_keying_material()); }

      void assert_key_material_set(bool predicate) const {
         if(!predicate) {
            throw_key_not_set_error();
         }
      }

   private:
      void throw_key_not_set_error() const;

      /**
      * Run the key schedule
      * @param key the key
      */
      virtual void key_schedule(std::span<const uint8_t> key) = 0;
};

}  // namespace Botan

#endif
