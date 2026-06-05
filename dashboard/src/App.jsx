import { useEffect, useState } from "react";
import "./App.css";

function App() {
  const [telemetry, setTelemetry] = useState(null);

  useEffect(() => {
    const fetchTelemetry = async () => {
      try {
        const response = await fetch("http://127.0.0.1:8000/api/telemetry");
        const data = await response.json();
        setTelemetry(data);
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

  return (
    <main className="container">
      <h1>ESP32-CAM Telemetry Dashboard</h1>

      <section className="card">
        <h2>Device Status</h2>
        <p>
          Status:{" "}
          <strong className={telemetry.connected ? "online" : "offline"}>
            {telemetry.connected ? "Online" : "Offline"}
          </strong>
        </p>
        <p>Device: {telemetry.device ?? "Unknown"}</p>
        <p>Uptime: {telemetry.uptime_ms ?? 0} ms</p>
        <p>Wi-Fi RSSI: {telemetry.wifi_rssi ?? "N/A"} dBm</p>
      </section>
    </main>
  );
}

export default App;