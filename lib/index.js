'use strict'
const { TTS } = require('bindings')('tts');

const fs = require('fs');
const path = require('path');
const rimraf = require("rimraf");

const base = path.resolve(__dirname, `./src/sdk/bin/msc/res/${process.platform}`);
const resource = {
    en: path.resolve(base, `./common_en.jet`),
    zh: path.resolve(base, `./common.jet`),
    xiaoyan: path.resolve(base, `./xiaoyan.jet`),
    xiaofeng: path.resolve(base, `./xiaofeng.jet`),
}

const default_res = [resource.xiaoyan, resource.zh]

const obj2str = (obj) => {
    /**
     * @param {obj}  需要转换的对象
     * @return {string} 返回key1=val1,key2=val2....格式
     */
    let w = Object.entries(obj);
    w.forEach((v, i) => { w[i] = v.join('=') });
    return w.join(',');
}

const generateSoures = (arr) => {
    return `tts_res_path = ${arr.filter(dir => fs.existsSync(dir)).map(dir => `fo|${dir}`).join(';')}`
}

const tts = new TTS();


tts.SetSources = (sources) => {
    if (!sources) throw new Error("资源必须为数组，项为各个资源的绝对路径");
    if (!sources.length) throw new Error("至少包含一个资源");
    tts.SetValue(generateSoures(sources));
}

tts.SetSources(default_res);

tts.login = (appid, sources) => {
    const login_params = `appid = ${appid}, work_dir = .`;
    if (sources) tts.SetSources(sources)
    return tts.TTSLogin(login_params);
}

tts.speak = (info = {}) => {
    const { text = "这是一条测试语音", filename = "sample.wav", params = {} } = info;
    const defalut_params = { engine_type: "local", voice_name: "xiaoyan", text_encoding: "UTF8", sample_rate: "16000", speed: "50", volume: "50", pitch: "50", rdn: "2" };
    let hasttspath = false;
    if (["xiaofeng"].includes(params.voice_name)) {
        default_res[0] = resource[params.voice_name];
        hasttspath = true;
    }
    if (params.lang) {
        hasttspath = true;
        default_res[1] = resource[params.lang];
        delete params.lang;
    }
    const synthetic_params = { ...defalut_params, ...params };
    if (hasttspath) tts.SetSources(default_res);
    const ttsInfo = tts.TextToSpeech({ text, filename, params: obj2str(synthetic_params) });
    if (ttsInfo.code === 0) {
        // const data = fs.readFileSync(path.resolve(`./${filename}`));
        // ttsInfo.data = data;
    }
    // del log and file
    // console.log(rimraf, path.resolve(`./${filename}`));
    rimraf(path.resolve(`./msc`), () => { })
    setTimeout(() => {
        rimraf(path.resolve(`./${filename}`), () => { })
    }, 100);
    return ttsInfo;
}

tts.GetSources = () => tts.GetValue();

module.exports = tts;
