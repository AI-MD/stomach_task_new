#include "Infer.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <math.h>

Infer::Infer(bool isGPU, const wchar_t* model_path, int deviceID)
{
	
	env = std::shared_ptr<Ort::Env>(new Ort::Env(ORT_LOGGING_LEVEL_WARNING, "test"));
	
	Ort::SessionOptions session_options;
	session_options.SetIntraOpNumThreads(1);
	
	OrtSessionOptionsAppendExecutionProvider_CUDA(session_options, 0);
	
	session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
	
	session = std::shared_ptr<Ort::Session>(new Ort::Session(*env, model_path, session_options));
	
	//std::cout << "Model Read Success" << std::endl;
}

void Infer::SetInputOutputSet()
{

	Ort::AllocatorWithDefaultOptions allocator;

	char* input_name = session->GetInputName(0, allocator);
	input_node_names.push_back(input_name);

	Ort::TypeInfo type_info = session->GetInputTypeInfo(0);
	auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
	input_node_dims = tensor_info.GetShape();

	output_node_names.push_back(session->GetOutputName(1, allocator));

}

void Infer::PrintInputNode()
{
	Ort::AllocatorWithDefaultOptions allocator;

	size_t num_input_nodes = session->GetInputCount();
	std::vector<const char*> input_node_names(num_input_nodes);
	std::vector<int64_t> input_node_dims;


	printf("Number of inputs = %zu\n", num_input_nodes);

	// iterate over all input nodes
	for (int i = 0; i < num_input_nodes; i++) {
		// print input node names
		char* input_name = session->GetInputName(i, allocator);
		printf("Input %d : name=%s\n", i, input_name);
		input_node_names[i] = input_name;

		// print input node types
		Ort::TypeInfo type_info = session->GetInputTypeInfo(i);
		auto tensor_info = type_info.GetTensorTypeAndShapeInfo();

		ONNXTensorElementDataType type = tensor_info.GetElementType();
		printf("Input %d : type=%d\n", i, type);

		// print input shapes/dims
		input_node_dims = tensor_info.GetShape();
		printf("Input %d : num_dims=%zu\n", i, input_node_dims.size());
		for (int j = 0; j < input_node_dims.size(); j++)
			printf("Input %d : dim %d=%jd\n", i, j, input_node_dims[j]);

		for (int j = 0; j < session->GetOutputCount(); j++)
		{
			char* output_name = session->GetOutputName(j, allocator);
			printf("output %d : name=%s\n", j, output_name);


			Ort::TypeInfo type_info = session->GetOutputTypeInfo(j);
			auto tensor_info = type_info.GetTensorTypeAndShapeInfo();

			ONNXTensorElementDataType type = tensor_info.GetElementType();
			printf("output %d : type=%d\n", j, type);

			std::vector<int64_t> output_node_dims = tensor_info.GetShape();
			printf("output %d : num_dims=%zu\n", j, output_node_dims.size());
			for (int k = 0; k < output_node_dims.size(); k++)
				printf("output %d : dim %d=%jd\n", j, k, output_node_dims[k]);

		}
	}
}

void Infer::GetOutput(std::vector<float>& input_tensor_values, const int clsNum)
{
	//std::cout << "GetOutput" << std::endl;
	// create input tensor object from data values
	auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

	Ort::Value input_tensor = Ort::Value::CreateTensor(memory_info, input_tensor_values.data(),
		input_tensor_values.size() * sizeof(float), input_node_dims.data(), 4);
	assert(input_tensor.IsTensor());

	// score model & input tensor, get back output tensor
	auto output_tensors = session->Run(Ort::RunOptions{ nullptr }, input_node_names.data(), &input_tensor, 1, output_node_names.data(), 1);
	assert(output_tensors.size() == 1 && output_tensors.front().IsTensor());

	// Get pointer to output tensor float values
	float* outputsArr = output_tensors.front().GetTensorMutableData<float>();

	outputs.assign(outputsArr, outputsArr + clsNum);

}

std::vector<float> Infer::Mat2Vec(cv::Mat& img, bool isColor, bool isPytorch)
{

	cv::Mat img_float;

	int channel = (isColor) ? 3 : 1;
	if (isColor)
	{
		img.convertTo(img_float, CV_32FC3);
	}
	else
	{
		img.convertTo(img_float, CV_32FC1);
	}

	img_float = img_float / 255.;


	std::vector<float> input_tensor_values;

	if (isPytorch)
	{
		std::vector<cv::Mat> split;
		cv::split(img_float, split);

		input_tensor_values.insert(std::end(input_tensor_values), (float*)split[0].data, (float*)split[0].data + split[0].total());
		input_tensor_values.insert(std::end(input_tensor_values), (float*)split[1].data, (float*)split[1].data + split[1].total());
		input_tensor_values.insert(std::end(input_tensor_values), (float*)split[2].data, (float*)split[2].data + split[2].total());
	}
	else
	{
		if (img_float.isContinuous())
		{
			input_tensor_values.assign((float*)img_float.data, (float*)img_float.data + img_float.total() * channel);
		}
		else {
			for (int i = 0; i < img_float.rows; ++i)
			{
				input_tensor_values.insert(input_tensor_values.begin(), img_float.ptr<float>(i), img_float.ptr<float>(i) + img_float.cols * channel);
			}
		}
	}
	return input_tensor_values;
}

std::vector<float> Infer::sigmoid(const std::vector <float>& m1) {


	const unsigned long VECTOR_SIZE = m1.size();
	std::vector <float> output(VECTOR_SIZE);


	for (unsigned i = 0; i != VECTOR_SIZE; ++i)
	{
		output[i] = 1 / (1 + exp(-m1[i]));
	}

	return output;
}

std::string Infer::GetPredictClass()
{
	return predict_class;
}

void Infer::AfterProcessing(const std::vector<const char*> class_name, const std::vector<float> cut_off)
{
	std::vector<float> results = sigmoid(outputs);
	bool check = false;
	int temp_index = 3;
	if (class_name.size() == 9)
		temp_index = 3;
	else
		temp_index = 4;
	/*std::cout << "result :";
	for (int i = 0; i < results.size(); ++i)
	{
		std::cout << results[i] << " ";
	}
	std::cout << std::endl;*/
	double prob = 0.0;
	for (int i = 0; i < results.size(); ++i)
	{
		if (i == 0)
		{
			if (results[i] >= 0.5)
			{
				predict_class = "X";
				check = true;
				break;
			}
		}

		else if (i > 0 && i < temp_index)
		{
			if (results[i] >= 0.97)
			{
				predict_class = class_name[i];
				check = true;
				break;

			}
		}

		else
		{
			if (results[i] >= cut_off[i - temp_index])
			{
				predict_class = class_name[i];
				check = true;
				
				break;

			}
		}
	}
	if (!check)
	{
		predict_class = "X";
	}
	std::stringstream prob_string;
	
	if (predict_class == "X")
	{
		std::string result_class = class_name[std::max_element(results.begin(), results.end()) - results.begin()];
		prob = floor(*std::max_element(results.begin(), results.end()) * 10000.f + 0.5) / 10000.f * 100;
		prob_string << prob;
	
		error_predict = "(" + result_class +" : " + prob_string.str()+"% )";
		
	}else
	{
		prob = floor(*std::max_element(results.begin(), results.end()) * 10000.f + 0.5) / 10000.f * 100;
		prob_string << prob;

		classProb = prob_string.str() + "%";
	}
	
		

	/*std::cout << "predict class : " << predict_class << std::endl;
	std::cout << std::endl;
	std::cout << "prob : " << classProb << std::endl;
	std::cout << std::endl;*/
	/*
		output 출력과 본 시스템의 후처리 알고리즘 분리
	*/
	/*std::cout << "predict class : " << predict_class << std::endl;
	std::cout << std::endl;*/
}

std::string Infer::GetErrorPred()
{
	return error_predict;
}
std::string Infer::getProb()
{
	
	return classProb;
}