# infer.py (TFLITE VERSION)

import pandas as pd
import numpy as np
import pickle
import os
import tensorflow as tf
from utils import file_to_windows

INFER_FILES = [
    "data/normal_20260217_181258.csv"
]

THRESHOLD = 0.5

RESULT_DIR = "result"
os.makedirs(RESULT_DIR, exist_ok=True)

# ==========================
# ⭐ TFLite 모델 로드
# ==========================
interpreter = tf.lite.Interpreter(
    model_path="model/motor_anomaly.tflite"
)
interpreter.allocate_tensors()

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

# scaler
scaler = pickle.load(open("model/scaler.pkl", "rb"))

def run_inference(filepath):
    feats, _, positions = file_to_windows(filepath)

    X = pd.DataFrame(feats).values.astype(np.float32)
    X = scaler.transform(X)

    scores = []

    for x in X:
        x_input = np.expand_dims(x, axis=0).astype(np.float32)

        interpreter.set_tensor(
            input_details[0]['index'],
            x_input
        )

        interpreter.invoke()

        output = interpreter.get_tensor(
            output_details[0]['index']
        )

        scores.append(output[0][0])

    score = np.array(scores)
    pred = (score > THRESHOLD).astype(int)

    result_df = pd.DataFrame({
        "window_start": positions,
        "anomaly_score": score,
        "pred_label": pred
    })

    filename = os.path.basename(filepath)
    save_name = filename.replace(".csv", "_result_tflite.csv")
    save_path = os.path.join(RESULT_DIR, save_name)

    result_df.to_csv(save_path, index=False)

    print(f"[INFER] {filepath} → {save_path}")

for f in INFER_FILES:
    run_inference(f)
