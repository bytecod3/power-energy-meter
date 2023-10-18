<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;

class SimulationController extends Controller
{
    //

    /**
     * Show simulationpage index
     */
    public function index(){
       return view('simulation.index');
    }
}
