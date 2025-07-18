/*
* UUID type
* (C) 2015,2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_UUID_H_
#define BOTAN_UUID_H_

#include <botan/types.h>
#include <string>
#include <vector>

BOTAN_DEPRECATED_HEADER("uuid.h")

namespace Botan {

class RandomNumberGenerator;

class BOTAN_UNSTABLE_API UUID final {
   public:
      /**
      * Create an uninitialized UUID object
      */
      UUID() = default;

      /**
      * Create a random UUID
      */
      BOTAN_FUTURE_EXPLICIT UUID(RandomNumberGenerator& rng);

      /**
      * Load a UUID from a 16 byte vector
      */
      BOTAN_FUTURE_EXPLICIT UUID(const std::vector<uint8_t>& blob);

      /**
      * Decode a UUID string
      */
      BOTAN_FUTURE_EXPLICIT UUID(std::string_view uuid_str);

      /**
      * Convert the UUID to a string
      */
      std::string to_string() const;

      const std::vector<uint8_t>& binary_value() const { return m_uuid; }

      bool operator==(const UUID& other) const { return m_uuid == other.m_uuid; }

      bool operator!=(const UUID& other) const { return !(*this == other); }

      bool is_valid() const { return m_uuid.size() == 16; }

   private:
      std::vector<uint8_t> m_uuid;
};

}  // namespace Botan

#endif
