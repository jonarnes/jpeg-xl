// Copyright (c) the JPEG XL Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstring>

#include "lib/extras/codec.h"
#include "lib/jxl/base/span.h"
#include "lib/jxl/codec_in_out.h"
#include "lib/jxl/dec_file.h"
#include "lib/jxl/enc_cache.h"
#include "lib/jxl/enc_file.h"

extern "C" {

/* NOTA BENE: see file history to uncover how to decode HDR JPEGs to pixels. */

/** Result: uint32_t 'size' followed by compressed image (JXL). */
uint8_t* jxlCompress(const uint8_t* data, size_t size) {
  jxl::PaddedBytes compressed;
  jxl::CodecInOut io;
  jxl::Codec input_codec;
  if (!jxl::SetFromBytes(jxl::Span<const uint8_t>(data, size), &io, nullptr,
                         &input_codec)) {
    return nullptr;
  }
  jxl::CompressParams params;
  jxl::PassesEncoderState passes_encoder_state;
  if (!jxl::EncodeFile(params, &io, &passes_encoder_state, &compressed, nullptr,
                       nullptr)) {
    return nullptr;
  }
  size_t compressed_size = compressed.size();
  uint8_t* result = reinterpret_cast<uint8_t*>(malloc(compressed_size + 4));
  uint32_t* meta = reinterpret_cast<uint32_t*>(result);
  meta[0] = compressed_size;
  memcpy(result + 4, compressed.data(), compressed_size);
  return result;
  return nullptr;
}

/** Result: uint32_t 'size' followed by decompressed image (JPG). */
uint8_t* jxlDecompress(const uint8_t* data, size_t size) {
  jxl::PaddedBytes decompressed;
  jxl::CodecInOut io;
  jxl::DecompressParams params;
  if (!jxl::DecodeFile(params, jxl::Span<const uint8_t>(data, size), &io,
                       nullptr, nullptr)) {
    return nullptr;
  }
  io.use_sjpeg = false;
  io.jpeg_quality = 100;
  if (!jxl::Encode(io, jxl::Codec::kJPG, io.Main().c_current(), 8,
                   &decompressed, nullptr)) {
    return nullptr;
  }
  size_t decompressed_size = decompressed.size();
  uint8_t* result = reinterpret_cast<uint8_t*>(malloc(decompressed_size + 4));
  uint32_t* meta = reinterpret_cast<uint32_t*>(result);
  meta[0] = decompressed_size;
  memcpy(result + 4, decompressed.data(), decompressed_size);
  return result;
}

}  // extern "C"
