/**
 * Copyright 2020 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_ROI_POOLING_H_
#define MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_ROI_POOLING_H_

#include <vector>
#include "src/lite_kernel.h"
#include "nnacl/fp32/roi_pooling.h"

namespace mindspore::kernel {
class ROIPoolingCPUKernel : public LiteKernel {
 public:
  ROIPoolingCPUKernel(OpParameter *parameter, const std::vector<lite::tensor::Tensor *> &inputs,
                      const std::vector<lite::tensor::Tensor *> &outputs, const lite::Context *ctx,
                      const mindspore::lite::PrimitiveC *primitive)
      : LiteKernel(parameter, inputs, outputs, ctx, primitive) {
    param_ = reinterpret_cast<ROIPoolingParameter *>(parameter);
  }
  ~ROIPoolingCPUKernel() override = default;

  int Init() override;
  int ReSize() override;
  int Run() override;
  int DoExecute(int task_id);

 private:
  float *in_ptr_;
  float *out_ptr_;
  float *roi_ptr_;
  ROIPoolingParameter *param_;
};
}  // namespace mindspore::kernel

#endif  // MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_REVERSE_H_
