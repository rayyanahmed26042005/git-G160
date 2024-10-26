/** @type {import('tailwindcss').Config} */
export default {
  darkMode: 'class',
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {colors: {
      'custom-blue': '#081229',
      // Add your custom color
    },},
  },
  
  plugins: [],
  
}