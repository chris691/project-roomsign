<?php

namespace App\Http\Controllers;

use App\Models\Lesson;
use Illuminate\Http\Request;

class RoomsignController extends Controller
{
    public function getLessonsForTheDay(Request $request){
        $fields= $request->validate([
            'weekday' => 'required',
            'roomnumber' => 'required',
        ]);

        $lesson = Lesson::where('roomnumber', $fields['roomnumber'])->where('day', $fields['weekday'])
                            ->get();
        $sortedLessons = $lesson->sortBy("starting_time")->values();
        $response = [
            'lessons' => $sortedLessons
        ];

        return response($response, 200);
    }
}
