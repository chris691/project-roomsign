import { readingText } from './main.js';

let isSpeaking = false;


export function vorlesen() {
    var synth = window.speechSynthesis;
    var utterance = new SpeechSynthesisUtterance(readingText);

    isSpeaking = true;

    synth.onvoiceschanged = function() {
        let voices = synth.getVoices();
        if (voices.length > 0) {
            utterance.voice = voices.find(voice => voice.lang.startsWith('de') || voice.lang.startsWith('en'));
        }
        utterance.pitch = 1;
        utterance.rate = 1;
        synth.speak(utterance);
    };

    let voices = synth.getVoices();
    if (voices.length > 0) {
        utterance.voice = voices.find(voice => voice.lang.startsWith('de') || voice.lang.startsWith('en'));
        synth.speak(utterance);
    }

    utterance.onend = function() {
        isSpeaking = false;
    };

    utterance.onerror = function() {
        isSpeaking = false;
    };
}


document.body.addEventListener('click', function() {
  if (!isSpeaking) {  
    vorlesen();       
  }
});
