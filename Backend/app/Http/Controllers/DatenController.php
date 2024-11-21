<?php

namespace App\Http\Controllers;
use Illuminate\Support\Facades\Log;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Http;
use Symfony\Component\DomCrawler\Crawler;
use App\Models\Lesson;

class DatenController extends Controller
{
    public function saveData()
{   
    $saveInfo = [];
    $rooms = config('rooms.roomnames');
    $changedRooms = []; 

foreach($rooms as $room){
    $code = $room;
    $semester = 'ws24';
   
    try {
        // Daten von externer Quelle abrufen
        $response = Http::get('https://m-server.fk5.hs-bremen.de/plan/raum.aspx', [
            'code' => $code,
            'semester' => $semester,
            'team' => '4'
        ]);

        if ($response->successful()) {
            $html = $response->body();
            $crawler = new Crawler($html);
            $skipFirstRow = true;
            $newLessons = [];

            
            $crawler->filter('#GridView_einsatzplan tr')->each(function ($row) use ($code, &$skipFirstRow, &$semester, &$newLessons) {
                if ($skipFirstRow) {
                    $skipFirstRow = false; 
                    return; 
                }

                $firstDiv = $row->filter('td div[title]')->first();
                if ($firstDiv->count() > 0) {
                    $lessonData = [];

                    $row->filter('td')->each(function ($cell) use (&$lessonData) {
                        $lessonData[] = $cell->text();
                    });

                    if (count($lessonData) == 9 && 
                        $lessonData[5] !== "\u00a0" && $lessonData[6] !== "\u00a0" && 
                        $lessonData[7] !== "\u00a0" && trim($lessonData[6]) !== "" && trim($lessonData[7]) !== "") {
                        
                        $calendar_week = empty($lessonData[8]) ? 'Alle Wochen' : $lessonData[8];
                        
                        // Nur gültige Wochentage verarbeiten
                        if(in_array($lessonData[5], ['Mo', 'Di', 'Mi', 'Do', 'Fr'])) {
                            // Neue Lektion in das Array hinzufügen
                            $newLessons[] = [
                                'module' => $lessonData[0],
                                'lecturer' => $lessonData[1],
                                'course_of_study' => $lessonData[2],
                                'note' => $lessonData[3] !== "&nbsp;" ? $lessonData[3] : null,
                                'sws' => $lessonData[4],
                                'day' => trim($lessonData[5]),
                                'starting_time' => trim($lessonData[6]),
                                'ending_time' => trim($lessonData[7]),
                                'calendar_week' => $calendar_week,
                                'roomnumber' => $code,
                                'semester' => $semester
                            ];
                        }
                    }
                }
            });

            $existingLessons = Lesson::where('roomnumber', $code)
            ->where('semester', $semester)
            ->get()->map(function ($lesson) {
                return [
                    'module' => $lesson->module,
                    'lecturer' => $lesson->lecturer,
                    'course_of_study' => $lesson->course_of_study,
                    'note' => $lesson->note,
                    'sws' => $lesson->sws,
                    'day' => $lesson->day,
                    'starting_time' => $lesson->starting_time,
                    'ending_time' => $lesson->ending_time,
                    'calendar_week' => $lesson->calendar_week,
                    'roomnumber' => $lesson->roomnumber,
                    'semester' => $lesson->semester
                ];
            })->toArray();

// Sortieren der Daten 
usort($newLessons, function($a, $b) {
return $a['module'] <=> $b['module'] ?: $a['day'] <=> $b['day'] ?: $a['starting_time'] <=> $b['starting_time'];
});

usort($existingLessons, function($a, $b) {
return $a['module'] <=> $b['module'] ?: $a['day'] <=> $b['day'] ?: $a['starting_time'] <=> $b['starting_time'];
});

// Vergleichen
if ($newLessons != $existingLessons) {
// Unterschiede gefunden:
Lesson::where('roomnumber', $code)
->where('semester', $semester)
->delete();

foreach ($newLessons as $lesson) {
Lesson::create($lesson);
}
$changedRooms[] = $code;
} else {
$status = 'Keine Änderung';
}
            $status = 'Erfolgreich';
        } else {
            $status = "Fehler";
        }
    } catch (\Exception $e) {
        return response()->json(['message' => 'Ein Fehler ist aufgetreten: ' . $e->getMessage()], 500);
    } 

    $saveInfo[$room] = $status;
}

// Rückgabe der Liste der geänderten Räume
return response()->json([
    'message' => $saveInfo,
    'changed_rooms' => $changedRooms
], 200);
}
}
