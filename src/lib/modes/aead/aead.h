/*
* Interface for AEAD modes
* (C) 2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AEAD_MODE_H_
#define BOTAN_AEAD_MODE_H_

#include <botan/cipher_mode.h>

#include <span>

namespace Botan {

/**
* Interface for AEAD (Authenticated Encryption with Associated Data)
* modes. These modes provide both encryption and message
* authentication, and can authenticate additional per-message data
* which is not included in the ciphertext (for instance a sequence
* number).
*/
class BOTAN_PUBLIC_API(2, 0) AEAD_Mode : public Cipher_Mode {
   public:
      /**
      * Create an AEAD mode
      * @param algo the algorithm to create
      * @param direction specify if this should be an encryption or decryption AEAD
      * @param provider optional specification for provider to use
      * @return an AEAD mode or a null pointer if not available
      */
      static std::unique_ptr<AEAD_Mode> create(std::string_view algo,
                                               Cipher_Dir direction,
                                               std::string_view provider = "");

      /**
      * Create an AEAD mode, or throw
      * @param algo the algorithm to create
      * @param direction specify if this should be an encryption or decryption AEAD
      * @param provider optional specification for provider to use
      * @return an AEAD mode, or throw an exception
      */
      static std::unique_ptr<AEAD_Mode> create_or_throw(std::string_view algo,
                                                        Cipher_Dir direction,
                                                        std::string_view provider = "");

      /**
      * Set associated data that is not included in the ciphertext but that
      * should be authenticated. Must be called after set_key() and before
      * start().
      *
      * Unless reset by another call, the associated data is kept between
      * messages. Thus, if the AD does not change, calling once (after
      * set_key()) is the optimum.
      *
      * @param ad the associated data
      */
      void set_associated_data(std::span<const uint8_t> ad) { set_associated_data_n(0, ad); }

      void set_associated_data(const uint8_t ad[], size_t ad_len) { set_associated_data(std::span(ad, ad_len)); }

      /**
      * Set associated data that is not included in the ciphertext but
      * that should be authenticated. Must be called after set_key() and
      * before start().
      *
      * Unless reset by another call, the associated data is kept
      * between messages. Thus, if the AD does not change, calling
      * once (after set_key()) is the optimum.
      *
      * Some AEADs (namely SIV) support multiple AD inputs. For
      * all other modes only nominal AD input 0 is supported; all
      * other values of idx will cause an exception.
      *
      * Derived AEADs must implement this. For AEADs where
      * `maximum_associated_data_inputs()` returns 1 (the default), the
      * @p idx must simply be ignored.
      *
      * @param idx which associated data to set
      * @param ad the associated data
      */
      virtual void set_associated_data_n(size_t idx, std::span<const uint8_t> ad) = 0;

      /**
      * Returns the maximum supported number of associated data inputs which
      * can be provided to set_associated_data_n
      *
      * If returns 0, then no associated data is supported.
      */
      virtual size_t maximum_associated_data_inputs() const { return 1; }

      /**
      * Most AEADs require the key to be set prior to setting the AD
      * A few allow the AD to be set even before the cipher is keyed.
      * Such ciphers would return false from this function.
      */
      virtual bool associated_data_requires_key() const { return true; }

      /**
      * Set associated data that is not included in the ciphertext but
      * that should be authenticated. Must be called after set_key() and
      * before start().
      *
      * See @ref set_associated_data().
      *
      * @param ad the associated data
      */
      template <typename Alloc>
      BOTAN_DEPRECATED("Simply use set_associated_data")
      void set_associated_data_vec(const std::vector<uint8_t, Alloc>& ad) {
         set_associated_data(ad);
      }

      /**
      * Set associated data that is not included in the ciphertext but
      * that should be authenticated. Must be called after set_key() and
      * before start().
      *
      * See @ref set_associated_data().
      *
      * @param ad the associated data
      */
      BOTAN_DEPRECATED("Use set_associated_data") void set_ad(std::span<const uint8_t> ad) { set_associated_data(ad); }

      /**
      * @return default AEAD nonce size (a commonly supported value among AEAD
      * modes, and large enough that random collisions are unlikely)
      */
      size_t default_nonce_length() const override { return 12; }
};

/**
* Get an AEAD mode by name (eg "AES-128/GCM" or "Serpent/EAX")
* @param name AEAD name
* @param direction Cipher_Dir::Encryption or Cipher_Dir::Decryption
*/
BOTAN_DEPRECATED("Use AEAD_Mode::create") inline AEAD_Mode* get_aead(std::string_view name, Cipher_Dir direction) {
   return AEAD_Mode::create(name, direction, "").release();
}

}  // namespace Botan

#endif
