/**
* Copyright 2023-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#include "InworldVAD.h"

#if 1

#include <iostream>
#include <vector>
#include <cstring>
#include <limits>
#include <chrono>
#include <memory>
#include <string>
#include "onnxruntime_cxx_api.h"
//#include "wav.h"
#include <cstdio>
#include <cstdarg>
#if __cplusplus < 201703L
#include <memory>
#endif

//#define __DEBUG_SPEECH_PROB___

class timestamp_t
{
public:
    int start;
    int end;

    // default + parameterized constructor
    timestamp_t(int start = -1, int end = -1)
        : start(start), end(end)
    {
    };

    // assignment operator modifies object, therefore non-const
    timestamp_t& operator=(const timestamp_t& a)
    {
        start = a.start;
        end = a.end;
        return *this;
    };

    // equality comparison. doesn't modify object. therefore const.
    bool operator==(const timestamp_t& a) const
    {
        return (start == a.start && end == a.end);
    };
    std::string c_str()
    {
        //return std::format("timestamp {:08d}, {:08d}", start, end);
        return format("{start:%08d,end:%08d}", start, end);
    };
private:

    std::string format(const char* fmt, ...)
    {
        char buf[256];

        va_list args;
        va_start(args, fmt);
        const auto r = std::vsnprintf(buf, sizeof buf, fmt, args);
        va_end(args);

        if (r < 0)
            // conversion failed
            return {};

        const size_t len = r;
        if (len < sizeof buf)
            // we fit in the buffer
            return { buf, len };

#if __cplusplus >= 201703L
        // C++17: Create a string and write to its underlying array
        std::string s(len, '\0');
        va_start(args, fmt);
        std::vsnprintf(s.data(), len + 1, fmt, args);
        va_end(args);

        return s;
#else
        // C++11 or C++14: We need to allocate scratch memory
        auto vbuf = std::unique_ptr<char[]>(new char[len + 1]);
        va_start(args, fmt);
        std::vsnprintf(vbuf.get(), len + 1, fmt, args);
        va_end(args);

        return { vbuf.get(), len };
#endif
    };
};


class VadIterator
{
private:
    // OnnxRuntime resources
    Ort::Env env;
    Ort::SessionOptions session_options;
    std::shared_ptr<Ort::Session> session = nullptr;
    Ort::AllocatorWithDefaultOptions allocator;
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPU);

private:
    void init_engine_threads(int inter_threads, int intra_threads)
    {
        // The method should be called in each thread/proc in multi-thread/proc work
        session_options.SetIntraOpNumThreads(intra_threads);
        session_options.SetInterOpNumThreads(inter_threads);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    };

    void init_onnx_model(const std::wstring& model_path)
    {
        // Init threads = 1 for 
        init_engine_threads(1, 1);
        // Load model
        session = std::make_shared<Ort::Session>(env, model_path.c_str(), session_options);
    };

    void reset_states()
    {
        // Call reset before each audio start
        std::memset(_h.data(), 0.0f, _h.size() * sizeof(float));
        std::memset(_c.data(), 0.0f, _c.size() * sizeof(float));
        triggered = false;
        temp_end = 0;
        current_sample = 0;

        prev_end = next_start = 0;

        speeches.clear();
        current_speech = timestamp_t();
    };

    void predict(const std::vector<float> &data)
    {
        // Infer
        // Create ort tensors
        input.assign(data.begin(), data.end());
        Ort::Value input_ort = Ort::Value::CreateTensor<float>(
            memory_info, input.data(), input.size(), input_node_dims, 2);
        Ort::Value sr_ort = Ort::Value::CreateTensor<int64_t>(
            memory_info, sr.data(), sr.size(), sr_node_dims, 1);
        Ort::Value h_ort = Ort::Value::CreateTensor<float>(
            memory_info, _h.data(), _h.size(), hc_node_dims, 3);
        Ort::Value c_ort = Ort::Value::CreateTensor<float>(
            memory_info, _c.data(), _c.size(), hc_node_dims, 3);

        // Clear and add inputs
        ort_inputs.clear();
        ort_inputs.emplace_back(std::move(input_ort));
        ort_inputs.emplace_back(std::move(sr_ort));
        ort_inputs.emplace_back(std::move(h_ort));
        ort_inputs.emplace_back(std::move(c_ort));

        // Infer
        ort_outputs = session->Run(
            Ort::RunOptions{nullptr},
            input_node_names.data(), ort_inputs.data(), ort_inputs.size(),
            output_node_names.data(), output_node_names.size());

        // Output probability & update h,c recursively
        float speech_prob = ort_outputs[0].GetTensorMutableData<float>()[0];
        float *hn = ort_outputs[1].GetTensorMutableData<float>();
        std::memcpy(_h.data(), hn, size_hc * sizeof(float));
        float *cn = ort_outputs[2].GetTensorMutableData<float>();
        std::memcpy(_c.data(), cn, size_hc * sizeof(float));

        probability = speech_prob;

        // Push forward sample index
        current_sample += window_size_samples;

        // Reset temp_end when > threshold 
        if ((speech_prob >= threshold))
        {
#ifdef __DEBUG_SPEECH_PROB___
            float speech = current_sample - window_size_samples; // minus window_size_samples to get precise start time point.
            printf("{    start: %.3f s (%.3f) %08d}\n", 1.0 * speech / sample_rate, speech_prob, current_sample- window_size_samples);
#endif //__DEBUG_SPEECH_PROB___
            if (temp_end != 0)
            {
                temp_end = 0;
                if (next_start < prev_end)
                    next_start = current_sample - window_size_samples;
            }
            if (triggered == false)
            {
                triggered = true;

                current_speech.start = current_sample - window_size_samples;
            }
            return;
        }

        if (
            (triggered == true)
            && ((current_sample - current_speech.start) > max_speech_samples)
            ) {
            if (prev_end > 0) {
                current_speech.end = prev_end;
                speeches.push_back(current_speech);
                current_speech = timestamp_t();
                
                // previously reached silence(< neg_thres) and is still not speech(< thres)
                if (next_start < prev_end)
                    triggered = false;
                else{
                    current_speech.start = next_start;
                }
                prev_end = 0;
                next_start = 0;
                temp_end = 0;

            }
            else{ 
                current_speech.end = current_sample;
                speeches.push_back(current_speech);
                current_speech = timestamp_t();
                prev_end = 0;
                next_start = 0;
                temp_end = 0;
                triggered = false;
            }
            return;

        }
        if ((speech_prob >= (threshold - 0.15)) && (speech_prob < threshold))
        {
            if (triggered) {
#ifdef __DEBUG_SPEECH_PROB___
                float speech = current_sample - window_size_samples; // minus window_size_samples to get precise start time point.
                printf("{ speeking: %.3f s (%.3f) %08d}\n", 1.0 * speech / sample_rate, speech_prob, current_sample - window_size_samples);
#endif //__DEBUG_SPEECH_PROB___
            }
            else {
#ifdef __DEBUG_SPEECH_PROB___
                float speech = current_sample - window_size_samples; // minus window_size_samples to get precise start time point.
                printf("{  silence: %.3f s (%.3f) %08d}\n", 1.0 * speech / sample_rate, speech_prob, current_sample - window_size_samples);
#endif //__DEBUG_SPEECH_PROB___
            }
            return;
        }


        // 4) End 
        if ((speech_prob < (threshold - 0.15)))
        {
#ifdef __DEBUG_SPEECH_PROB___
            float speech = current_sample - window_size_samples - speech_pad_samples; // minus window_size_samples to get precise start time point.
            printf("{      end: %.3f s (%.3f) %08d}\n", 1.0 * speech / sample_rate, speech_prob, current_sample - window_size_samples);
#endif //__DEBUG_SPEECH_PROB___
            if (triggered == true)
            {
                if (temp_end == 0)
                {
                    temp_end = current_sample;
                }
                if ((int)(current_sample - temp_end) > min_silence_samples_at_max_speech)
                    prev_end = temp_end;
                // a. silence < min_slience_samples, continue speaking 
                if ((int)(current_sample - temp_end) < min_silence_samples)
                {

                }
                // b. silence >= min_slience_samples, end speaking
                else
                {
                    current_speech.end = temp_end;
                    if (current_speech.end - current_speech.start > min_speech_samples)
                    {
                        speeches.push_back(current_speech);
                        current_speech = timestamp_t();
                        prev_end = 0;
                        next_start = 0;
                        temp_end = 0;
                        triggered = false;
                    }
                }
            }
            else {
                // may first windows see end state.
            }
            return;
        }
    };
public:
    void process(const std::vector<float>& input_wav)
    {
        //reset_states();

        audio_length_samples = input_wav.size();

        for (int j = 0; j < audio_length_samples; j += window_size_samples)
        {
            if (j + window_size_samples > audio_length_samples)
                break;
            std::vector<float> r{ &input_wav[0] + j, &input_wav[0] + j + window_size_samples };
            predict(r);
        }

        if (current_speech.start >= 0) {
            current_speech.end = audio_length_samples;
            speeches.push_back(current_speech);
            current_speech = timestamp_t();
            prev_end = 0;
            next_start = 0;
            temp_end = 0;
            triggered = false;
        }
    };

    void process(const std::vector<float>& input_wav, std::vector<float>& output_wav)
    {
        process(input_wav);
        collect_chunks(input_wav, output_wav);
    }

    void collect_chunks(const std::vector<float>& input_wav, std::vector<float>& output_wav)
    {
        output_wav.clear();
        for (int i = 0; i < speeches.size(); i++) {
#ifdef __DEBUG_SPEECH_PROB___
            std::cout << speeches[i].c_str() << std::endl;
#endif //#ifdef __DEBUG_SPEECH_PROB___
            std::vector<float> slice(&input_wav[speeches[i].start], &input_wav[speeches[i].end]);
            output_wav.insert(output_wav.end(),slice.begin(),slice.end());
        }
    };

    const std::vector<timestamp_t> get_speech_timestamps() const
    {
        return speeches;
    }

    float get_speech_probability() const
    {
        return probability;
    }

    void drop_chunks(const std::vector<float>& input_wav, std::vector<float>& output_wav)
    {
        output_wav.clear();
        int current_start = 0;
        for (int i = 0; i < speeches.size(); i++) {

            std::vector<float> slice(&input_wav[current_start],&input_wav[speeches[i].start]);
            output_wav.insert(output_wav.end(), slice.begin(), slice.end());
            current_start = speeches[i].end;
        }

        std::vector<float> slice(&input_wav[current_start], &input_wav[input_wav.size()]);
        output_wav.insert(output_wav.end(), slice.begin(), slice.end());
    };

private:
    // model config
    int64_t window_size_samples;  // Assign when init, support 256 512 768 for 8k; 512 1024 1536 for 16k.
    int sample_rate;  //Assign when init support 16000 or 8000      
    int sr_per_ms;   // Assign when init, support 8 or 16
    float threshold; 
    int min_silence_samples; // sr_per_ms * #ms
    int min_silence_samples_at_max_speech; // sr_per_ms * #98
    int min_speech_samples; // sr_per_ms * #ms
    float max_speech_samples;
    int speech_pad_samples; // usually a 
    int audio_length_samples;

    // model states
    bool triggered = false;
    unsigned int temp_end = 0;
    unsigned int current_sample = 0;
    float probability = 0.0f;;
    // MAX 4294967295 samples / 8sample per ms / 1000 / 60 = 8947 minutes  
    int prev_end;
    int next_start = 0;

    //Output timestamp
    std::vector<timestamp_t> speeches;
    timestamp_t current_speech;


    // Onnx model
    // Inputs
    std::vector<Ort::Value> ort_inputs;
    
    std::vector<const char *> input_node_names = {"input", "sr", "h", "c"};
    std::vector<float> input;
    std::vector<int64_t> sr;
    unsigned int size_hc = 2 * 1 * 64; // It's FIXED.
    std::vector<float> _h;
    std::vector<float> _c;

    int64_t input_node_dims[2] = {}; 
    const int64_t sr_node_dims[1] = {1};
    const int64_t hc_node_dims[3] = {2, 1, 64};

    // Outputs
    std::vector<Ort::Value> ort_outputs;
    std::vector<const char *> output_node_names = {"output", "hn", "cn"};

public:
    // Construction
    VadIterator(const std::wstring ModelPath,
        int Sample_rate = 16000, int windows_frame_size = 64,
        float Threshold = 0.5, int min_silence_duration_ms = 0,
        int speech_pad_ms = 64, int min_speech_duration_ms = 64,
        float max_speech_duration_s = std::numeric_limits<float>::infinity())
    {
        init_onnx_model(ModelPath);
        threshold = Threshold;
        sample_rate = Sample_rate;
        sr_per_ms = sample_rate / 1000;

        window_size_samples = windows_frame_size * sr_per_ms;

        min_speech_samples = sr_per_ms * min_speech_duration_ms;
        speech_pad_samples = sr_per_ms * speech_pad_ms;

        max_speech_samples = (
            sample_rate * max_speech_duration_s
            - window_size_samples
            - 2 * speech_pad_samples
            );

        min_silence_samples = sr_per_ms * min_silence_duration_ms;
        min_silence_samples_at_max_speech = sr_per_ms * 98;

        input.resize(window_size_samples);
        input_node_dims[0] = 1;
        input_node_dims[1] = window_size_samples;

        _h.resize(size_hc);
        _c.resize(size_hc);
        sr.resize(1);
        sr[0] = sample_rate;
    };
};

std::unique_ptr<VadIterator> g_VadIterator;

Inworld::VAD::VAD(const std::string& Model)
{
    g_VadIterator = std::make_unique<VadIterator>(L"C:/Projects/inworld/TestEmpty52/DEV-services_ml-hosting_silero_vad_10_27_2022.onnx");
}

Inworld::VAD::~VAD()
{
    g_VadIterator.reset();
}

float Inworld::VAD::ProcessAudioChunk(const std::vector<float>& AudioData)
{
    if (!g_VadIterator)
    {
        return 0.f;
    }

    g_VadIterator->process(AudioData);
    return g_VadIterator->get_speech_probability();
}

#endif