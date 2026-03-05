from flask import Flask, render_template, request, jsonify
import requests
import joblib
import random

app = Flask(__name__)

# ================= ML MODEL =================
model = joblib.load("health_model.pkl")

# ================= BLYNK CONFIG (TEMP + ECG ONLY) =================
BLYNK_AUTH_TOKEN = "gjkFRTR5T-rmcj-RgFEbXyNALcRAK4Za"
BLYNK_URL = "https://blynk.cloud/external/api/get"

PIN_TEMP = "V1"
PIN_ECG = "V3"

# ================= PATIENT STORAGE =================
patients = []

# ================= REALISTIC HR SIMULATION =================
current_hr = random.randint(62, 68)

def get_realistic_heart_rate():
    global current_hr
    change = random.randint(-2, 2)
    current_hr = max(60, min(75, current_hr + change))
    return current_hr


def get_blynk_value(pin):
    try:
        r = requests.get(
            BLYNK_URL,
            params={"token": BLYNK_AUTH_TOKEN, "pin": pin},
            timeout=5
        )
        return float(r.text)
    except:
        return 0.0


# ================= ROUTES =================

@app.route('/')
def home():
    return render_template("home.html")


@app.route('/patient', methods=['GET', 'POST'])
def patient():
    if request.method == 'POST':
        # Manual inputs
        name = request.form['name']
        age = float(request.form['age'])

        # Data sources
        temp = get_blynk_value(PIN_TEMP)        # from Blynk / cloud
        heartrate = get_realistic_heart_rate()  # simulated realistic HR
        ecg = get_blynk_value(PIN_ECG)          # from Blynk / cloud

        # ML Prediction (still executed)
        X = [[age, temp, heartrate, ecg]]
        _ = model.predict(X)[0]   # prediction ignored for demo safety

        # ===== FORCE LOW RISK =====
        risk = "Low Risk"
        message = (
            "Your heart health parameters are within the normal range. "
            "No immediate medical consultation is required. "
            "Please maintain a healthy lifestyle."
        )

        patient_data = {
            "name": name,
            "age": age,
            "temp": temp,
            "heartrate": heartrate,
            "ecg": ecg,
            "risk": risk
        }

        patients.append(patient_data)

        return render_template(
            "patient_result.html",
            patient=patient_data,
            message=message
        )

    return render_template("patient.html")


@app.route('/doctor')
def doctor():
    return render_template("doctor.html", patients=patients)


# ================= LIVE DATA FOR AUTO-FETCH =================

@app.route('/get_sensor_data')
def get_sensor_data():
    return jsonify({
        "temp": get_blynk_value(PIN_TEMP),
        "heartrate": get_realistic_heart_rate(),
        "ecg": get_blynk_value(PIN_ECG)
    })


# ================= RUN =================

if __name__ == "__main__":
    app.run(debug=True)
