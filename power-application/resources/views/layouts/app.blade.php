<!DOCTYPE html>
<html lang="{{ str_replace('_', '-', app()->getLocale()) }}">
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">

        <title>@yield('title')</title>

        <!-- Fonts -->
        <link rel="preconnect" href="https://fonts.bunny.net">
        <link href="https://fonts.bunny.net/css?family=figtree:400,600&display=swap" rel="stylesheet" />
        <script src="https://use.fontawesome.com/fd981f6249.js"></script>

        <!-- Styles -->
        @vite('resources/css/app.css')
    </head>

    <body class="antialiased bg-gray-100">
        @include('includes.header')
        <div>
            @yield('main-content')      
        </div>

        <script src="../path/to/flowbite/dist/flowbite.min.js"></script>
    </body>

</html>
