const express = require('express');
const cors    = require('cors');
const path    = require('path');

const app  = express();
const PORT = 3000;

app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

/* Latest sensor reading — in memory */
let latestData = {
  temperature: null,
  humidity:    null,
  bpm:         null,
  fingerOn:    false,
  dhtOk:       false,
  timestamp:   null
};

/* ESP32 POSTs here every second */
app.post('/data', (req, res) => {
  const { temperature, humidity, bpm, fingerOn, dhtOk } = req.body;
  latestData = {
    temperature: temperature !== -1 ? temperature : null,
    humidity:    humidity    !== -1 ? humidity    : null,
    bpm:         bpm         !== -1 ? bpm         : null,
    fingerOn,
    dhtOk,
    timestamp: new Date().toISOString()
  };
  console.log(`[${new Date().toLocaleTimeString()}] T:${temperature}°C  H:${humidity}%  BPM:${bpm}  Finger:${fingerOn}`);
  res.json({ ok: true });
});

/* Dashboard polls this */
app.get('/api/data', (req, res) => {
  res.json(latestData);
});

app.listen(PORT, '0.0.0.0', () => {
  console.log(`\n🌡️  HeatShield AI Dashboard`);
  console.log(`   Server running at http://localhost:${PORT}`);
  console.log(`   ESP32 should POST to http://<YOUR_PC_IP>:${PORT}/data\n`);
});