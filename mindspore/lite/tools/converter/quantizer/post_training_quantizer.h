/**
 * Copyright 2019 Huawei Technologies Co., Ltd
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

#ifndef POSTRAINING_QUANTIZER_H
#define POSTRAINING_QUANTIZER_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <cfloat>
#include <map>
#include <utility>
#include "src/lite_session.h"
#include "tools/converter/quantizer/quantizer.h"
#include "tools/converter/converter.h"
#include "include/ms_tensor.h"

namespace mindspore {
namespace lite {
namespace quant {
class Calibrator;

struct MaxMin {
 public:
  float min;
  float max;
};

const char kMethodMaxMin[] = "MAX_MIN";
const char kMethodKL[] = "KL";
constexpr int kDefaultBinNumber = 2048;

struct ConfigParam {
  std::string image_path;
  uint32_t batch_count{100};
  std::string method_x{kMethodKL};
  uint32_t thread_num{1};
};

class PostTrainingQuantizer : public Quantizer {
 public:
  PostTrainingQuantizer(FuncGraphPtr graph, std::string path, int bit_num, TypeId target_type = kNumberTypeInt8,
                        bool per_channel = true);

  STATUS DoQuantize(FuncGraphPtr funcGraph) override;

  size_t bit_num;
  int quant_max{INT8_MAX};
  int quant_min{INT8_MIN};

 private:
  bool per_channel_;

  TypeId target_type_{kNumberTypeInt8};

  std::unique_ptr<Calibrator> calibrator_;

  mindspore::lite::LiteSession *session_;

  STATUS PreProcess();

  STATUS CheckTensorVec(const std::string &node_name,
                        const std::vector<mindspore::tensor::MSTensor *> &tensor_vec) const;

  STATUS DoInference();

  STATUS UpdateDivergInverval();

  STATUS CollectDataFrequency();

  STATUS ComputeThreshold();

  STATUS QuantNode();

  STATUS DoQuantInput(double scale, int32_t zeropoint, struct MaxMin *max_min, std::shared_ptr<PrimitiveC>);
  STATUS DoQuantOutput(double scale, int32_t zeropoint, struct MaxMin *max_min, std::shared_ptr<PrimitiveC>);

  STATUS DoWeightQuant(AnfNodePtr weight, std::shared_ptr<PrimitiveC> primitive_c, bool perchannel, bool depthwise);

  STATUS DoBiasQuant(AnfNodePtr bias, std::shared_ptr<PrimitiveC> primitive_c);
};

struct DivergInfo {
  std::vector<float> histogram;
  CNodePtr cnode;
  int bin_num;
  float interval = 0;
  float max;
  float min;
  float best_T = 0.0f;
  size_t bit_num;
  int quant_max = 255;
  int quant_min = 0;
  std::string method_x = kMethodKL;

  DivergInfo(CNodePtr cnode, int bins, size_t bits, int quant_max, int quant_min, const std::string &method_x) {
    this->method_x = method_x;
    this->cnode = cnode;
    this->bin_num = bins;
    this->bit_num = bits;
    histogram.resize(bin_num);
    max = -FLT_MAX;
    min = FLT_MAX;
    this->quant_max = quant_max;
    this->quant_min = quant_min;
    std::fill(histogram.begin(), histogram.end(), 1.0e-7);
  }

  STATUS RecordMaxValue(const std::vector<float> &datas);

  void UpdateInterval();

  STATUS UpdateHistogram(const std::vector<float> &data);

  void DumpHistogram();

  STATUS ComputeThreshold();

  std::pair<CNodePtr, float> GetScale();

  std::pair<CNodePtr, int32_t> GetZeropoint();
};

class Calibrator {
 public:
  explicit Calibrator(std::string path, size_t bit_num, int quant_max, int quant_min);

  ~Calibrator() = default;

  STATUS ReadConfig();

  STATUS CollectImages();

  STATUS GenerateInputData(int index, mindspore::tensor::MSTensor *tensor) const;

  size_t GetBatchNum() const { return images_.size(); }

  uint32_t GetThreadNum() const { return config_param_.thread_num; }

  std::string GetMethodX() const { return config_param_.method_x; }

  STATUS AddQuantizedOp(CNodePtr node);

  STATUS RecordMaxValue(const std::string &op_name, const std::vector<float> &data,
                        std::unordered_map<std::string, std::unique_ptr<DivergInfo>> *diverg_info);

  STATUS UpdateDivergInverval(std::unordered_map<std::string, std::unique_ptr<DivergInfo>> *diverg_info);

  STATUS UpdateDataFrequency(const std::string &op_name, const std::vector<float> &data,
                             std::unordered_map<std::string, std::unique_ptr<DivergInfo>> *diverg_info);
  void Dump();

  STATUS ComputeThreshold();

  std::unordered_map<CNodePtr, float> GetScale(
    std::unordered_map<std::string, std::unique_ptr<DivergInfo>> *diverg_info);

  std::unordered_map<CNodePtr, int32_t> GetZeropoint(
    std::unordered_map<std::string, std::unique_ptr<DivergInfo>> *diverg_info);

  std::map<CNodePtr, MaxMin> GetMinMax(std::unordered_map<std::string, std::unique_ptr<DivergInfo>> *diverg_info);

  std::unordered_map<std::string, std::unique_ptr<DivergInfo>> *GetInputDivergInfo();

  std::unordered_map<std::string, std::unique_ptr<DivergInfo>> *GetOutputDivergInfo();

 private:
  std::vector<std::string> images_;

  std::string config_path_;

  ConfigParam config_param_;

  std::unordered_map<std::string, std::unique_ptr<DivergInfo>> input_diverg_info_;

  std::unordered_map<std::string, std::unique_ptr<DivergInfo>> output_diverg_info_;

  size_t bit_num_;
  int quant_max_;
  int quant_min_;

  void AddImage(std::string file);
};
}  // namespace quant
}  // namespace lite
}  // namespace mindspore
#endif  // POSTRAINING_QUANTIZER_H
