import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

export default defineConfig({
  plugins: [react()],
  css: {
    postcss: './postcss.config.js',
    server: {
      historyApiFallback: true,},  // This points to your PostCSS config
  }
})