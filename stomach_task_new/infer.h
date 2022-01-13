#pragma once
#include "onnxruntime_cxx_api.h"
#include <cuda_provider_factory.h>
#include "opencv2/opencv.hpp"

class Infer
{
private:
	std::shared_ptr<Ort::Env> env;
	std::shared_ptr<Ort::Session> session;
	//Ort::Env *env = nullptr;
	//Ort::Session *session = nullptr;
	std::vector<const char*> input_node_names;
	std::vector<int64_t> input_node_dims;
	std::vector<const char*> output_node_names;
	std::string predict_class;
	std::string classProb;
	std::vector<float> outputs;
	std::string error_predict;
public:
	Infer(bool isGPU, const wchar_t* model_path, int deviceID = 0);
	void PrintInputNode();
	void SetInputOutputSet();
	void GetOutput(std::vector<float>& input_tensor_values, const int clsNum);
	std::vector<float>  Mat2Vec(cv::Mat& img, bool isColor = true, bool isPytorch = true);
	std::vector<float> sigmoid(const std::vector<float>& m1);
	std::string GetPredictClass();
	void AfterProcessing(const std::vector<const char*> class_name, const std::vector<float> cut_off);
	std::string getProb();
	std::string GetErrorPred();
};

