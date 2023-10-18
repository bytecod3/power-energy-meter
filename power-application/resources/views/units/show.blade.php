@extends('layouts.app')

@section('main-content')

<div>

    <div class="grid grid-cols-60-40 p-2 gap-2">

        <div class="flex flex-col">

            <div class="w-full text-white font-bold p-2 bg-gradient-to-b from-gray-200 via-gray-400 to-gray-600">Device status</div>

            <div>
                <div class="p-2 grid grid-cols-3 gap-4">
                    {{-- Activity --}}
                    <div class="flex flex-col border border-green-300 p-4 bg-white rounded-md">
                        <div class="font-bold text-gray-700 border-b border-gray-300">
                            Status
                        </div>
        
                        <div class="text-green-600 shadow-sm font-extrabold">
                            <span class="text-5xl ">ACTIVE</span>
                        </div>
        
                    </div>
                    {{-- Wattage --}}
                    <div class="flex flex-col border border-gray-300 p-4 bg-white rounded-md">
                        <div class="font-bold text-gray-700 border-b border-gray-300">
                            Power consumed
                        </div>
        
                        <div class="text-gray-500 font-extrabold">
                            <span class="text-5xl ">34.5 </span>
                            <span>kWh</span>
                        </div>
        
                    </div>
        
                    {{-- Current --}}
                    <div class="flex flex-col border border-gray-300 p-4 bg-white rounded-md">
                        <div class="font-bold text-gray-700 border-b border-gray-300">
                            Instantaneous Current
                        </div>
        
                        <div class="text-gray-500 font-extrabold">
                            <span class="text-5xl ">5 </span>
                            <span>A</span>
                        </div>
        
                    </div>
        
                    {{-- Voltage -RMS --}}
                    <div class="flex flex-col border border-gray-300 p-4 bg-white rounded-md">
                        <div class="font-bold text-gray-700 border-b border-gray-300">
                            RMS Voltage
                        </div>
        
                        <div class="text-gray-500 font-extrabold">
                            <span class="text-5xl ">230</span>
                            <span>Vrms</span>
                        </div>
        
                    </div>
        
                    {{-- Units --}}
                    <div class="flex flex-col border border-gray-300 p-4 bg-white rounded-md">
                        <div class="font-bold text-gray-700 border-b border-gray-300">
                            Remaining Units
                        </div>
        
                        <div class="text-gray-500 font-extrabold">
                            <span class="text-5xl ">45</span>
                            <span>Units</span>
                        </div>
        
                    </div>
        
                    {{-- Uptime --}}
                    <div class="flex flex-col border border-gray-300 p-4 bg-white rounded-md">
                        <div class="font-bold text-gray-700 border-b border-gray-300">
                            Device uptime
                        </div>
        
                        <div class="text-gray-500 font-extrabold">
                            <span class="text-5xl ">233</span>
                            <span>Hrs</span>
                        </div>
        
                    </div>
        
                    {{-- Last paid --}}
                    <div class="flex flex-col border border-gray-300 p-4 bg-white rounded-md">
                        <div class="font-bold text-gray-700 border-b border-gray-300">
                            Last paid
                        </div>
        
                        <div class="text-gray-500 font-extrabold">
                            <span class="text-2xl ">20-22-12</span>
                        </div>
        
                    </div>
                    
                </div>
                
            </div>
            
        </div>

       
        <div class="flex flex-col">
            <div class="w-full text-white font-bold p-2 bg-gradient-to-b from-gray-200 via-gray-400 to-gray-600">Device charts</div>

            <div class="p-2">

            </div>
        </div>
        

    </div>
    
    {{-- Sumary table --}}
    <div class="flex flex-col p-2">
        <div class="w-full text-white font-bold p-2 bg-gradient-to-b from-gray-200 via-gray-400 to-gray-600">Summary information</div>

        {{-- Summary table  --}}
        <div>
            

                <!-- This is an example component -->

            <div class="flex flex-col">
            <div class="overflow-x-auto shadow-md sm:rounded-sm">
                <div class="inline-block min-w-full align-middle">
                    <div class="overflow-hidden ">
                        <table class="min-w-full divide-y divide-gray-200 table-fixed dark:divide-gray-700">
                            <thead class="bg-gray-100 dark:bg-gray-700">
                                <tr>
                                   
                                    <th scope="col" class="py-3 px-6 text-xs font-medium tracking-wider text-left text-gray-700 uppercase dark:text-gray-400">
                                        Value
                                    </th>
                                    <th scope="col" class="py-3 px-6 text-xs font-medium tracking-wider text-left text-gray-700 uppercase dark:text-gray-400">
                                        Data
                                    </th>
                                  
                                   
                                </tr>
                            </thead>
                            <tbody class="bg-white divide-y divide-gray-200 dark:bg-gray-800 dark:divide-gray-700">
                                <tr class="hover:bg-gray-100 dark:hover:bg-gray-700">
                                    
                                    <td class="py-4 px-6 text-sm font-medium text-gray-900 whitespace-nowrap dark:text-white">Device ID</td>
                                    <td class="py-4 px-6 text-sm font-medium text-gray-500 whitespace-nowrap dark:text-white">34</td>
                                    
                                </tr>
                                <tr class="hover:bg-gray-100 dark:hover:bg-gray-700">
                                  
                                    <td class="py-4 px-6 text-sm font-medium text-gray-900 whitespace-nowrap dark:text-white">Customer ID</td>
                                    <td class="py-4 px-6 text-sm font-medium text-gray-500 whitespace-nowrap dark:text-white">123jgjg-g</td>
                             
                                </tr>
                                <tr class="hover:bg-gray-100 dark:hover:bg-gray-700">
                               
                                    <td class="py-4 px-6 text-sm font-medium text-gray-900 whitespace-nowrap dark:text-white">Amount paid last</td>
                                    <td class="py-4 px-6 text-sm font-medium text-gray-500 whitespace-nowrap dark:text-white">KES. 5500</td>
                                 
                                </tr>
                                <tr class="hover:bg-gray-100 dark:hover:bg-gray-700">
                                 
                                    <td class="py-4 px-6 text-sm font-medium text-gray-900 whitespace-nowrap dark:text-white">House No</td>
                                    <td class="py-4 px-6 text-sm font-medium text-gray-500 whitespace-nowrap dark:text-white">G11</td>
                            
                                </tr>
                          
                            </tbody>
                        </table>
                    </div>
                </div>
            </div>
            
            </div>
            </div>

    </div>
    </div>

</div>

@endsection