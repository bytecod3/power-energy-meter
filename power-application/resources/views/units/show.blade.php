@extends('layouts.app')

@section('main-content')

<div>

    <div class="grid grid-cols-60-40 p-2 gap-2">

        <div class="flex flex-col">

            <div class="w-full text-white font-bold p-2 bg-gradient-to-b from-gray-200 via-gray-400 to-gray-600">Device status</div>

            <div>
                <div class="p-2 grid grid-cols-3 gap-4">
                    {{-- Activity --}}
                    <div class="flex flex-col border bg-green-400  p-4 rounded-md text-white">
                        <div class="font-bold text-white border-b border-gray-300">
                            Status
                        </div>

                        <div class="text-white shadow-sm font-extrabold">
                            <span class="text-5xl ">ON</span>
                        </div>

                    </div>

                    {{-- Wattage --}}
                    <div class="text-white flex flex-col border border-gray-300 p-4 bg-blue-900 rounded-md">
                        <div class="font-bold border-b border-gray-300">
                            Power consumed
                        </div>

                        <div class="font-extrabold">
                            <span id="power_div" class="text-5xl "> </span>
                            <span>kWh</span>
                        </div>

                    </div>

                    {{-- Current --}}
                    <div class="flex flex-col border bg-green-400  p-4 rounded-md text-white">
                        <div class="font-bold border-b border-gray-300">
                            Instantaneous Current
                        </div>

                        <div class="font-extrabold">
                            <span id="current_div" class="text-5xl "> </span>
                            <span>A</span>
                        </div>

                    </div>

                    {{-- Voltage -RMS --}}
                    <div class="text-white flex flex-col border border-gray-300 p-4 bg-blue-900 rounded-md">
                        <div class="font-bold  border-b border-gray-300">
                            RMS Voltage
                        </div>

                        <div class="font-extrabold">
                            <span class="text-5xl ">230</span>
                            <span>Vrms</span>
                        </div>

                    </div>

                    {{-- cumulative power --}}
                    <div class="text-white flex flex-col border border-gray-300 p-4 bg-blue-900 rounded-md">
                        <div class="font-bold  border-b border-gray-300">
                            Cumulative power
                        </div>

                        <div class="font-extrabold">
                            <span id="cumulative_power" class="text-5xl "></span>
                            <span>kWh</span>
                        </div>

                    </div>

                    {{-- Units --}}
                    <div class="flex flex-col border bg-green-400  p-4 rounded-md text-white">
                        <div class="font-bold border-b">
                            Remaining Units
                        </div>

                        <div class=" font-extrabold">
                            <span id="units_div" class="text-5xl "></span>
                            <span>Units</span>
                        </div>

                    </div>

                    {{-- Total money spent --}}
                    <div class="flex flex-col border bg-green-400  p-4 rounded-md text-white">
                        <div class="font-bold border-b">
                            Amount spent
                        </div>

                        <div class=" font-extrabold">
                            <span id="amount_div" class="text-5xl "></span>
                            <span>KES</span>
                        </div>

                    </div>


                </div>

            </div>

        </div>

        <div class="flex flex-col">
            <div id="test" class=" w-full text-white font-bold p-2 bg-gradient-to-b from-gray-200 via-gray-400 to-gray-600">Log info</div>

            <div id="submsg" class="p-2">

            </div>
        </div>


    </div>

    {{-- Summary table --}}
    <div class="flex flex-col p-2">
        <div class="w-full text-white font-bold p-2 bg-gradient-to-b from-gray-200 via-gray-400 to-gray-600">Customer information</div>

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
                                    <td class="py-4 px-6 text-sm font-medium text-gray-500 whitespace-nowrap dark:text-white">M01-1234-5678</td>

                                </tr>
                                {{-- <tr class="hover:bg-gray-100 dark:hover:bg-gray-700">

                                    <td class="py-4 px-6 text-sm font-medium text-gray-900 whitespace-nowrap dark:text-white">Customer ID</td>
                                    <td class="py-4 px-6 text-sm font-medium text-gray-500 whitespace-nowrap dark:text-white">123jgjg-g</td>

                                </tr> --}}
                                <tr class="hover:bg-gray-100 dark:hover:bg-gray-700">

                                    <td class="py-4 px-6 text-sm font-medium text-gray-900 whitespace-nowrap dark:text-white">Amount paid last</td>
                                    <td class="py-4 px-6 text-sm font-medium text-gray-500 whitespace-nowrap dark:text-white">KES. 550</td>

                                </tr>
                                <tr class="hover:bg-gray-100 dark:hover:bg-gray-700">

                                    <td class="py-4 px-6 text-sm font-medium text-gray-900 whitespace-nowrap dark:text-white">House No</td>
                                    <td class="py-4 px-6 text-sm font-medium text-gray-500 whitespace-nowrap dark:text-white">A12</td>

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
