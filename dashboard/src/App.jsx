import { useEffect, useState } from "react";
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  Tooltip,
  ResponsiveContainer,
  CartesianGrid,
} from "recharts";
import "./App.css";

function App() {
  const [telemetry, setTelemetry] = useState(null);
  const [history, setHistory] = useState([]);
  const [page, setPage] = useState("dashboard");

  useEffect(() => {
    const fetchTelemetry = async () => {
      try {
        const response = await fetch("http://127.0.0.1:8000/api/telemetry");
        const data = await response.json();

        setTelemetry(data);

        if (data.connected) {
          setHistory((prev) => {
            const nextPoint = {
              time: new Date().toLocaleTimeString(),
              device: data.device ?? "Unknown",
              rssi: data.wifi_rssi,
              uptime: Math.floor((data.uptime_ms ?? 0) / 1000),
              status: "Online",
            };

            return [...prev.slice(-49), nextPoint];
          });
        }
      } catch (error) {
        console.error("Failed to fetch telemetry:", error);
      }
    };

    fetchTelemetry();
    const interval = setInterval(fetchTelemetry, 3000);

    return () => clearInterval(interval);
  }, []);

  if (!telemetry) {
    return <h1>Loading telemetry...</h1>;
  }

  const isOnline = telemetry.connected;
  const rssiDisplay = isOnline ? `${telemetry.wifi_rssi} dBm` : "--";
  const uptimeDisplay = isOnline
    ? `${Math.floor((telemetry.uptime_ms ?? 0) / 1000)} sec`
    : "--";

  return (
    <div className="dashboard">
      <header className="header">
        <h1>ESP32 Telemetry Dashboard</h1>
        <p>Live Device Monitoring</p>

        <nav className="nav-tabs">
          <button
            className={page === "dashboard" ? "active-tab" : ""}
            onClick={() => setPage("dashboard")}
          >
            Dashboard
          </button>

          <button
            className={page === "history" ? "active-tab" : ""}
            onClick={() => setPage("history")}
          >
            Signal Log
          </button>
        </nav>
      </header>

      {page === "dashboard" && (
        <>
          <div className="metrics-grid">
            <div className="metric-card">
              <span className="metric-label">Status</span>
              <span className={`metric-value ${isOnline ? "online" : "offline"}`}>
                ● {isOnline ? "Online" : "Offline"}
              </span>
            </div>

            <div className="metric-card">
              <span className="metric-label">Signal Strength</span>
              <span className="metric-value">{rssiDisplay}</span>
            </div>

            <div className="metric-card">
              <span className="metric-label">Uptime</span>
              <span className="metric-value">{uptimeDisplay}</span>
            </div>
          </div>

          <div className="details-card">
            <h2>Signal Strength History</h2>

            <ResponsiveContainer width="100%" height={300}>
              <LineChart data={history}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="time" />
                <YAxis domain={[-100, -30]} />
                <Tooltip />
                <Line
                  type="monotone"
                  dataKey="rssi"
                  stroke="#38bdf8"
                  strokeWidth={3}
                  dot={false}
                />
              </LineChart>
            </ResponsiveContainer>
          </div>

          <div className="details-card">
            <h2>Device Information</h2>

            <div className="detail-row">
              <span>Device</span>
              <span>{isOnline ? telemetry.device : "--"}</span>
            </div>

            <div className="detail-row">
              <span>MQTT Connection</span>
              <span>{isOnline ? "Connected" : "Disconnected"}</span>
            </div>

            <div className="detail-row">
              <span>RSSI</span>
              <span>{rssiDisplay}</span>
            </div>
            <div className="detail-row">
              <span>IP Address</span>
              <span>{isOnline ? telemetry.ip : "--"}</span>
            </div>

            <div className="detail-row">
              <span>MAC Address</span>
              <span>{isOnline ? telemetry.mac : "--"}</span>
            </div>

            <div className="detail-row">
              <span>Free Heap</span>
              <span>{isOnline ? `${Math.round(telemetry.free_heap / 1024)} KB` : "--"}</span>
            </div>

            <div className="detail-row">
              <span>Minimum Heap</span>
              <span>{isOnline ? `${Math.round(telemetry.min_heap / 1024)} KB` : "--"}</span>
            </div>

            <div className="detail-row">
              <span>CPU Frequency</span>
              <span>{isOnline ? `${telemetry.cpu_freq} MHz` : "--"}</span>
            </div>

            <div className="detail-row">
              <span>Wi-Fi Channel</span>
              <span>{isOnline ? telemetry.wifi_channel : "--"}</span>
            </div>

            <div className="detail-row">
              <span>Camera Active</span>
              <span>{isOnline ? String(telemetry.camera_active) : "--"}</span>
            </div>

            <div className="detail-row">
              <span>Frames Captured</span>
              <span>{isOnline ? telemetry.frames_captured : "--"}</span>
            </div>

            <div className="detail-row">
              <span>Capture Time</span>
              <span>{isOnline ? `${telemetry.capture_time_ms} ms` : "--"}</span>
            </div>

            <div className="detail-row">
              <span>Image Size</span>
              <span>{isOnline ? `${Math.round(telemetry.image_size / 1024)} KB` : "--"}</span>
            </div>

            <div className="detail-row">
              <span>Messages Sent</span>
              <span>{isOnline ? telemetry.messages_sent : "--"}</span>
            </div>

            <div className="detail-row">
              <span>MQTT Reconnects</span>
              <span>{isOnline ? telemetry.mqtt_reconnects : "--"}</span>
            </div>
          </div>
        </>
      )}

      {page === "history" && (
        <div className="details-card">
          <h2>Signal History Log</h2>

          <table className="history-table">
            <thead>
              <tr>
                <th>Time</th>
                <th>Device</th>
                <th>Status</th>
                <th>RSSI</th>
                <th>Uptime</th>
              </tr>
            </thead>

            <tbody>
              {[...history].reverse().map((entry, index) => (
                <tr key={index}>
                  <td>{entry.time}</td>
                  <td>{entry.device}</td>
                  <td>{entry.status}</td>
                  <td>{entry.rssi} dBm</td>
                  <td>{entry.uptime} sec</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
    </div>
  );
}

export default App;