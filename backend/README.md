

Setting up python backend

run requirements

```
cd backend
python -m venv .venv
.venv\Scripts\activate
pip install -r requirements.txt
```

to run backend

```
cd ~/ESP32_Telemetry_Dashboard/ESP32-Telemetry-Dashboard/backend
python -m uvicorn app.main:app --reload
```

which also starts a fastAPI at http://127.0.0.1:8000.

You can test it at http://127.0.0.1:8000/api/telemetry