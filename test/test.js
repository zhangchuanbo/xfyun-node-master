const tts = require('../lib/index.js');

const loginInfo = tts.login("591d86fc");
console.log(loginInfo);

const speakInfo = tts.speak({ text: "hello", filename: "sample.wav" })
console.log(speakInfo);
