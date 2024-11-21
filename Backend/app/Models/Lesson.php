<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class Lesson extends Model
{
    use HasFactory;

    protected $fillable = [
        'roomnumber',
        'day',
        'starting_time',
        'ending_time',
        'lecturer',
        'module',
        'note',
        'sws',
        'course_of_study',
        'calendar_week',
        'semester'
    ];

}
