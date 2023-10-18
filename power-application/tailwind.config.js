/** @type {import('tailwindcss').Config} */
export default {
    content: [
        "./resources/**/*.blade.php",
        "./resources/**/*.js",
        "./resources/**/*.vue",
        "./node_modules/flowbite/**/*.js"
    ],
    theme: {
        extend: {
            // 70 - 30 column view
            gridTemplateColumns: {
                '60-40': '60% 40%'
            }
        },
    },
    plugins: [
        require('flowbite/plugin')
    ],
}