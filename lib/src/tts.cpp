/*
* 语音合成（Text To Speech，TTS）技术能够自动将任意文字实时转换为连续的
* 自然语音，是一种能够在任何时间、任何地点，向任何人提供语音信息服务的
* 高效便捷手段，非常符合信息时代海量数据、动态更新和个性化查询的需求。
*/

#include <napi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "qtts.h"
#include "msp_cmn.h"
#include "msp_errors.h"

typedef int SR_DWORD;
typedef short int SR_WORD ;

/* wav音频头部格式 */
typedef struct _wave_pcm_hdr
{
	char            riff[4];                // = "RIFF"
	int				size_8;                 // = FileSize - 8
	char            wave[4];                // = "WAVE"
	char            fmt[4];                 // = "fmt "
	int				fmt_size;				// = 下一个结构体的大小 : 16

	short int       format_tag;             // = PCM : 1
	short int       channels;               // = 通道数 : 1
	int				samples_per_sec;        // = 采样率 : 8000 | 6000 | 11025 | 16000
	int				avg_bytes_per_sec;      // = 每秒字节数 : samples_per_sec * bits_per_sample / 8
	short int       block_align;            // = 每采样点字节数 : wBitsPerSample / 8
	short int       bits_per_sample;        // = 量化比特数: 8 | 16

	char            data[4];                // = "data";
	int				data_size;              // = 纯数据长度 : FileSize - 44 
} wave_pcm_hdr;

/* 默认wav音频头部数据 */
wave_pcm_hdr default_wav_hdr = 
{
	{ 'R', 'I', 'F', 'F' },
	0,
	{'W', 'A', 'V', 'E'},
	{'f', 'm', 't', ' '},
	16,
	1,
	1,
	16000,
	32000,
	2,
	16,
	{'d', 'a', 't', 'a'},
	0  
};

/* 文本合成 */
int text_to_speech(const char* src_text, const char* des_path, const char* params)
{
	int          ret          = -1;
	FILE*        fp           = NULL;
	const char*  sessionID    = NULL;
	unsigned int audio_len    = 0;
	wave_pcm_hdr wav_hdr      = default_wav_hdr;
	int          synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;

	if (NULL == src_text || NULL == des_path)
	{
		printf("params is error!\n");
		return ret;
	}
	fp = fopen(des_path, "wb");
	if (NULL == fp)
	{
		printf("open %s error.\n", des_path);
		return ret;
	}
	/* 开始合成 */
	sessionID = QTTSSessionBegin(params, &ret);
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSSessionBegin failed, error code: %d.\n", ret);
		fclose(fp);
		return ret;
	}
	ret = QTTSTextPut(sessionID, src_text, (unsigned int)strlen(src_text), NULL);
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSTextPut failed, error code: %d.\n",ret);
		QTTSSessionEnd(sessionID, "TextPutError");
		fclose(fp);
		return ret;
	}
	fwrite(&wav_hdr, sizeof(wav_hdr) ,1, fp); //添加wav音频头，使用采样率为16000
	while (1) 
	{
		/* 获取合成音频 */
		const void* data = QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
		if (MSP_SUCCESS != ret)
			break;
		if (NULL != data)
		{
			fwrite(data, audio_len, 1, fp);
		    wav_hdr.data_size += audio_len; //计算data_size大小
		}
		if (MSP_TTS_FLAG_DATA_END == synth_status)
			break;
	}
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSAudioGet failed, error code: %d.\n",ret);
		QTTSSessionEnd(sessionID, "AudioGetError");
		fclose(fp);
		return ret;
	}
	/* 修正wav文件头数据的大小 */
	wav_hdr.size_8 += wav_hdr.data_size + (sizeof(wav_hdr) - 8);
	
	/* 将修正过的数据写回文件头部,音频文件为wav格式 */
	fseek(fp, 4, 0);
	fwrite(&wav_hdr.size_8,sizeof(wav_hdr.size_8), 1, fp); //写入size_8的值
	fseek(fp, 40, 0); //将文件指针偏移到存储data_size值的位置
	fwrite(&wav_hdr.data_size,sizeof(wav_hdr.data_size), 1, fp); //写入data_size的值
	fclose(fp);
	fp = NULL;
	/* 合成完毕 */
	ret = QTTSSessionEnd(sessionID, "Normal");
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSSessionEnd failed, error code: %d.\n",ret);
	}

	return ret;
}

class TTS : public Napi::ObjectWrap<TTS> {
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    TTS(const Napi::CallbackInfo& info);
    static Napi::Value CreateNewItem(const Napi::CallbackInfo& info);
    
  private:
    std::string tts_res_path;
    Napi::Value GetValue(const Napi::CallbackInfo& info);
    Napi::Value SetValue(const Napi::CallbackInfo& info);
    Napi::Value TTSLogin(const Napi::CallbackInfo& info);
    Napi::Value GetVersion(const Napi::CallbackInfo& info);
    Napi::Value TextToSpeech(const Napi::CallbackInfo& info);
};

Napi::Object TTS::Init(Napi::Env env, Napi::Object exports) {
    // This method is used to hook the accessor and method callbacks
    Napi::Function func = DefineClass(env, "TTS", {
        InstanceMethod<&TTS::GetValue>("GetValue", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&TTS::SetValue>("SetValue", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&TTS::TTSLogin>("TTSLogin", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&TTS::GetVersion>("GetVersion", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        InstanceMethod<&TTS::TextToSpeech>("TextToSpeech", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
        StaticMethod<&TTS::CreateNewItem>("CreateNewItem", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
    });

    Napi::FunctionReference* constructor = new Napi::FunctionReference();

    // Create a persistent reference to the class constructor. This will allow
    // a function called on a class prototype and a function
    // called on instance of a class to be distinguished from each other.
    *constructor = Napi::Persistent(func);
    exports.Set("TTS", func);

    // Store the constructor as the add-on instance data. This will allow this
    // add-on to support multiple instances of itself running on multiple worker
    // threads, as well as multiple instances of itself running in different
    // contexts on the same thread.
    //
    // By default, the value set on the environment here will be destroyed when
    // the add-on is unloaded using the `delete` operator, but it is also
    // possible to supply a custom deleter.
    env.SetInstanceData<Napi::FunctionReference>(constructor);

    return exports;
}

TTS::TTS(const Napi::CallbackInfo& info) :
    Napi::ObjectWrap<TTS>(info) {
  Napi::Env env = info.Env();
  if (!info.IsConstructCall())
  {
    Napi::TypeError::New(env, "TTS function can only be used as a constructor").ThrowAsJavaScriptException();
    return;
  }
}

Napi::Value TTS::GetValue(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    return Napi::String::New(env, this->tts_res_path);
}

Napi::Value TTS::SetValue(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    this->tts_res_path = info[0].As<Napi::String>().Utf8Value();
    return this->GetValue(info);
}

Napi::Value TTS::TTSLogin(const Napi::CallbackInfo &info)
{

    Napi::Env env = info.Env();
	Napi::Object objOut = Napi::Object::New(env);
    std::string str = info[0].As<Napi::String>().Utf8Value();
	MSPLogout();
	int ret = MSP_SUCCESS;
    const char* login_params  = str.c_str(); //登录参数,appid与msc库绑定,请勿随意改动
	ret = MSPLogin(NULL, NULL, login_params); //第一个参数是用户名，第二个参数是密码，第三个参数是登录参数，用户名和密码可在http://www.xfyun.cn注册获取
	objOut.Set("code",ret);
	if (MSP_SUCCESS != ret){
        objOut.Set("message","TTSLogin failed");
    }
    else {
        objOut.Set("message","TTSLogin success");
    }
	return objOut;
}

Napi::Value TTS::GetVersion(const Napi::CallbackInfo &info)
{

    Napi::Env env = info.Env();
    std::string str = info[0].As<Napi::String>().Utf8Value();
    const char* param  = str.c_str(); 
	const int g = MSP_ERROR_FAIL;
	int *errorCode = const_cast<int*>(&g);//去掉const常量const属性
	return Napi::String::New(env, MSPGetVersion(param, errorCode));;
}

Napi::Value TTS::TextToSpeech(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
    Napi::Object params = info[0].As<Napi::Object>();
	Napi::Object objOut = Napi::Object::New(env);

    // default params
    // "engine_type = purextts,voice_name=xiaoyan, text_encoding = UTF8, sample_rate = 16000, speed = 50, volume = 50, pitch = 50, rdn = 2"
    std::string temp = params.Get("params").As<Napi::String>().Utf8Value();
    const char* synthetic_params = temp.c_str();
    const char* catStr = ",";
    const char* res_path = this->tts_res_path.c_str();
    char *session_begin_params = (char *) malloc(strlen(synthetic_params) + strlen(catStr) + strlen(res_path));
    strcat(strcat(strcpy(session_begin_params,synthetic_params),catStr),res_path);
    // get params
    std::string tmp_filename = params.Get("filename").As<Napi::String>().Utf8Value();
    const char* filename             = tmp_filename.c_str(); //合成的语音文件名称

    std::string tmp_text = params.Get("text").As<Napi::String>().Utf8Value();
	const char* text                 = tmp_text.c_str(); //合成文本
	/* 文本合成 */
	int ret = text_to_speech(text, filename, session_begin_params);
    objOut.Set("code",ret);
	if (MSP_SUCCESS != ret){
        objOut.Set("message","QTTSTextPut failed");
    }
    else {
        objOut.Set("message","QTTSTextPut success");
    }
    return objOut;
}

// Initialize native add-on
Napi::Object Init (Napi::Env env, Napi::Object exports) {
    TTS::Init(env, exports);
    return exports;
}

// Create a new item using the constructor stored during Init.
Napi::Value TTS::CreateNewItem(const Napi::CallbackInfo& info) {
  // Retrieve the instance data we stored during `Init()`. We only stored the
  // constructor there, so we retrieve it here to create a new instance of the
  // JS class the constructor represents.
  Napi::FunctionReference* constructor =
      info.Env().GetInstanceData<Napi::FunctionReference>();
  return constructor->New({ Napi::Number::New(info.Env(), 42) });
}

// Register and initialize native add-on
NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init);
