/*
* (C) 2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "fuzzers.h"

#include <botan/hex.h>
#include <botan/internal/oaep.h>

namespace {

Botan::CT::Option<size_t> ref_oaep_unpad(const uint8_t in[], size_t len, const std::vector<uint8_t>& Phash) {
   const size_t hlen = Phash.size();

   if(len < 2 * hlen + 1) {
      return Botan::CT::Option<size_t>();
   }

   for(size_t i = hlen; i != 2 * hlen; ++i) {
      if(in[i] != Phash[i - hlen]) {
         return Botan::CT::Option<size_t>();
      }
   }

   for(size_t i = 2 * hlen; i != len; ++i) {
      if(in[i] != 0x00 && in[i] != 0x01) {
         return Botan::CT::Option<size_t>();
      }

      if(in[i] == 0x01) {
         return Botan::CT::Option<size_t>(i + 1);
      }
   }

   return Botan::CT::Option<size_t>();
}

}  // namespace

void fuzz(const uint8_t in[], size_t len) {
   static const std::vector<uint8_t> Phash = {1, 2, 3, 4};

   auto lib_idx = Botan::oaep_find_delim(std::span{in, len}, std::span{Phash});

   auto ref_idx = ref_oaep_unpad(in, len, Phash);

   if(lib_idx.has_value().as_bool() && ref_idx.has_value().as_bool()) {
      FUZZER_ASSERT_EQUAL(lib_idx.value(), ref_idx.value());
   } else if(lib_idx.has_value().as_bool() && !ref_idx.has_value().as_bool()) {
      FUZZER_WRITE_AND_CRASH("Ref accepted but lib rejected\n");
   } else if(!lib_idx.has_value().as_bool() && ref_idx.has_value().as_bool()) {
      FUZZER_WRITE_AND_CRASH("Lib accepted but ref rejected\n");
   }
}
