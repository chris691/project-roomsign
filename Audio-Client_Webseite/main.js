import { vorlesen } from './textToSpeech.js';

// Ausgeschriebene Wochentage
var fullWeekdays = {
    'So': 'Sonntag',
    'Mo': 'Montag',
    'Di': 'Dienstag',
    'Mi': 'Mittwoch',
    'Do': 'Donnerstag',
    'Fr': 'Freitag',
    'Sa': 'Samstag'
};

// Abgekürzte Wochentage
var weekdays = ['So', 'Mo', 'Di', 'Mi', 'Do', 'Fr', 'Sa'];

// Aktuelles Datum und Uhrzeit erhalten
var currentDate = new Date();
var weekdayNumber = currentDate.getDay();
var currentWeekday = weekdays[weekdayNumber];
var raumnutzung = "Hörsaal" 
var fullWeekday = fullWeekdays[currentWeekday];

var day = currentDate.getDate();
var month = currentDate.getMonth() + 1;  
var year = currentDate.getFullYear();
var hours = currentDate.getHours();
var minutes = currentDate.getMinutes().toString().padStart(2, '0');
var modulNr = 0;

// Auslesen der Tag-ID aus der URL
function getTagIdFromUrl() {
    var url = window.location.href;
    var queryString = url.split('?')[1];
    var params = new URLSearchParams(queryString);
    return params.get('id');
}

var tagId = getTagIdFromUrl();
document.getElementById('room').innerText = tagId + " "+ raumnutzung;
document.getElementById('day-headline').innerText = "Stundenplan für " + fullWeekday + " den " + day +"."+month+"."+year;
document.getElementById('timeAndDate').innerText = hours+":"+minutes +" Uhr"


export var readingText = `Stundenplan für ${fullWeekday}, den ${day}.${month}.${year}, im Raum ${tagId}. ${raumnutzung}\nEs ist aktuell ${hours}:${minutes} Uhr.\n`;

var apiUrl = 'https://roomsign-api.schultz-tech.de/api/lessonForTheDay?roomnumber=' + tagId + '&weekday=' + currentWeekday;

fetch(apiUrl)
    .then(response => response.json())
    .then(data => {
        var lessonInfoDiv = document.getElementById('lessonInfo');
        var lessons = data.lessons;

        if (lessons.length > 0) {
            var lessonHtml = '<h2></h2>';
            var counter = 1;

            lessons.forEach((lesson) => {
                modulNr = getModuleNumber(lesson.starting_time.slice(0,-3)) 
                lessonHtml += `<p><strong>${modulNr}.</strong></p>`;
                lessonHtml += `<p><strong>Startzeit:</strong> ${lesson.starting_time.slice(0, -3)} Uhr</p>`;
                lessonHtml += `<p><strong>Endzeit:</strong> ${lesson.ending_time.slice(0, -3)} Uhr</p>`;
                lessonHtml += `<p><strong>Modul:</strong> ${lesson.module}</p>`;
                lessonHtml += `<p><strong>Dozent:</strong> ${lesson.lecturer}</p>`;
                lessonHtml += `<p><strong>Studiengang:</strong> ${lesson.course_of_study}</p>`;
                lessonHtml += '<hr>'; 

                // String zum vorlesen
                readingText += `${modulNr}. Modul von ${lesson.starting_time.slice(0, -3)} bis 
                                ${lesson.ending_time.slice(0, -3)} Uhr.\nModul: ${lesson.module},\nDozent: 
                                ${lesson.lecturer},\nStudiengang: ${lesson.course_of_study}\n`;
                counter++;
            });

            lessonInfoDiv.innerHTML = lessonHtml;
            readingText +="Zum Wiederholen auf den Bildschirm tippen."
        } else {
            // Wenn es an dem Tag keine Vorlesungen gibt. 
            readingText += "Heute sind keine Lektionen geplant.";
            lessonInfoDiv.innerHTML = '<p>Heute sind keine Lektionen geplant.</p>';
        }

        vorlesen();

    })
    .catch(error => {
        console.error('Error fetching data:', error);
        readingText = "Es ist ein Fehler bei der Abfrage der Lektionen aufgetreten.";
        vorlesen(); 
    });

function getModuleNumber(starting_time){
    switch(starting_time){
        case "08:00":
        return 1;
        case "08:45":
        return 1;
        case "09:45":
        return 2;
        case "11:30":
        return 3;
        case "13:30":
        return 4;
        case "15:15":
        return 5;
        case "17:00":
        return 6;
        case "18:45":
        return 7;
        case "20:15":
        return 8;
    }

}
