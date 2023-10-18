<?php

use App\Http\Controllers\UnitController;
use Illuminate\Support\Facades\Route;

/*
|--------------------------------------------------------------------------
| Web Routes
|--------------------------------------------------------------------------
|
| Here is where you can register web routes for your application. These
| routes are loaded by the RouteServiceProvider and all of them will
| be assigned to the "web" middleware group. Make something great!
|
*/

Route::get('/', function () {
    return view('units.all');
});

Route::get('/data', function(){
    return "Data page";
});


Route::get('/unit/{id}', [UnitController::class, 'show']);

// simulation routes
Route::get('/simulate/', [SimulationController::class, 'index']);