// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <vector>
#include "core/common/common.h"
#include "core/framework/op_kernel.h"
#include "contrib_ops/cpu/bert/attention_common.h"

namespace onnxruntime {
namespace contrib {

class AttentionBase {
 public:
  // This check function is specifically used in cuda
  Status CheckInputs(const TensorShape& input_shape,
                     const TensorShape* weights_shape,
                     const TensorShape& bias_shape,
                     const Tensor*& mask_index,  // Dummy mask of shape (1 or batch_size, 1) will be updated to nullptr.
                     const Tensor* past,
                     const Tensor* extra_add_qk,
                     const Tensor* key,
                     const Tensor* value,
                     void* parameters,
                     const int max_threads_per_block) const;

  Tensor* GetPresent(OpKernelContext* context,
                     const Tensor* past,
                     int batch_size,
                     int head_size,
                     int sequence_length,
                     int& past_sequence_length) const;

 protected:
  AttentionBase(const OpKernelInfo& info, bool require_same_hidden_size, bool require_weights) {
    int64_t num_heads = 0;
    ORT_ENFORCE(info.GetAttr("num_heads", &num_heads).IsOK() && num_heads > 0);
    num_heads_ = static_cast<int>(num_heads);

    is_unidirectional_ = info.GetAttrOrDefault<int64_t>("unidirectional", 0) == 1;

    if (!info.GetAttrs<int64_t>("qkv_hidden_sizes", qkv_hidden_sizes_).IsOK()) {
      qkv_hidden_sizes_.clear();
    }

    require_same_hidden_size_ = require_same_hidden_size;
    require_weights_ = require_weights;
  }

  Status CheckMask(const Tensor* mask_index,
                   bool& is_dummy,                // output: whether the mask is dummy with shape (1 or batch_size, 1)
                   int64_t& max_sequence_length,  // output: max_sequence_length when mask_index is 4D tensor
                   int64_t batch_size,
                   int64_t sequence_length,
                   int64_t total_sequence_length) const;

  Status CheckInputs(const TensorShape& input_shape,
                     const TensorShape* weights_shape,
                     const TensorShape& bias_shape,
                     const Tensor*& mask_index,  // Dummy mask of shape (1 or batch_size, 1) will be updated to nullptr.
                     const Tensor* past,
                     const Tensor* extra_add_qk,
                     const Tensor* key,
                     const Tensor* value,
                     void* parameters) const;

  int num_heads_;                          // number of attention heads
  bool is_unidirectional_;                 // whether every token can only attend to previous tokens.
  std::vector<int64_t> qkv_hidden_sizes_;  // Q, K, V hidden sizes parsed from the qkv_hidden_sizes attribute.
  bool require_same_hidden_size_;          // whether the implementation supports different hidden sizes of Q/K/V.
  bool require_weights_;                   // whether the implementation requires weights for Q/K/V.
};

}  // namespace contrib
}  // namespace onnxruntime
