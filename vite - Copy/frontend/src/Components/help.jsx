import React from 'react'
import "./Help.css";

function Help() {
  return (
    <div className='glass text'>
    <h1 className='underline'>Stuck? Welcome to the Help Page for your IoT-based automated Home! Here’s a quick guide to get started and troubleshoot common issues.</h1>
    
    <h2>Features :-</h2>
    <ul><li className='text2 font-bold mb-2'>Device Control: Use the mobile app, web dashboard, or voice assistant to control connected devices like lights and fans.</li>
    <li className='text2 font-bold mb-2'>Manual Override: Control devices manually using physical buttons, which override app commands temporarily.</li>
   <li className='text2 font-bold mb-2'> Real-time Monitoring: View the status of all devices and sensors through the app or dashboard.</li>
    <li className='text2 font-bold mb-2'>Automation: Set rules based on sensor data (e.g., turn on the fan when the temperature is high).</li>
    <h3>Troubleshooting</h3>
    <li className='text2 font-bold mb-2'>
    Device Not Responding: Check WiFi connection and ensure the Atlas server is online. Restart your device if needed.</li>
    <li className='text2 font-bold mb-2'> Manual Override Issues: Press the physical button again to reset to manual mode, or use the app to regain automated control.</li>
    <li className='text2 font-bold mb-2'>Connection Errors: If disconnected from WiFi, the system will reconnect automatically. Ensure stable internet for seamless operation.</li></ul>
    <h3>FAQs</h3>
    <h4>How do I add a new device?</h4>  <li className='text2 font-bold mb-2'>Use the Atlas app to set up and assign virtual pins for new devices.</li>
    <h4>Can I automate multiple devices? </h4><li className='text2 font-bold mb-2'>Yes! Use the app to set automation rules for each device based on sensor readings or time of day. </li>
   <h4>For further assistance, please contact support</h4>
    <button className='button'>BACK</button></div>
    
  )
}

export default Help