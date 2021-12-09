const tts = require('../lib/index.js');

const loginInfo = tts.login("4beb12d6");
console.log(loginInfo);

const speakInfo = tts.speak({ text: "hello", filename: "sample.wav" })
console.log(speakInfo);
