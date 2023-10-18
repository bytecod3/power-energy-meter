@extends('layouts.app')
@section('title', 'Power App - Home')

@section('main-content')
        <div class="p-10">
            {{-- info box --}}
            <div>
                
            </div>

            <div class="flex items-center p-4 mb-4 text-sm text-green-800 border border-green-300 rounded-lg bg-green-50 dark:bg-gray-800 dark:text-green-400 dark:border-green-800" role="alert">
                <svg class="flex-shrink-0 inline w-4 h-4 mr-3" aria-hidden="true" xmlns="http://www.w3.org/2000/svg" fill="currentColor" viewBox="0 0 20 20">
                  <path d="M10 .5a9.5 9.5 0 1 0 9.5 9.5A9.51 9.51 0 0 0 10 .5ZM9.5 4a1.5 1.5 0 1 1 0 3 1.5 1.5 0 0 1 0-3ZM12 15H8a1 1 0 0 1 0-2h1v-3H8a1 1 0 0 1 0-2h2a1 1 0 0 1 1 1v4h1a1 1 0 0 1 0 2Z"/>
                </svg>
                <span class="sr-only">Info</span>
                <div>
                  <span class="font-medium">Click on a single unit below to view its details</span>
                </div>
              </div>

            <div class=" grid grid-cols-4">
                <a href="/unit/1" class="bg-green-600 text-white rounded-md p-4">
                    <div class="">
                        <div class="font-bold">
                            <div class="text-xl">Unit 1</div>
                            <div>House No: <span>G11</span></div>
                            <div class="text-sm">Unit ID: <span>jjeje-jdwen</span></div>
                        </div>
                    </div>
                </a>
                 
            </div>
        </div>

@endsection