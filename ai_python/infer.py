# infer.py
import pandas as pd
import numpy as np
import pickle
import os
from tensorflow import keras
from utils import file_to_windows

INFER_FILES = [
    "data/normal_20260223_213852_part1.csv",   # ← 새 파일
    "data/defect_20260223_221151_part1.csv"    # ← 새 파일
]

THRESHOLD = 0.5   # 필요시 조정

# ===========================
# 결과 저장 폴더
# ===========================
RESULT_DIR = "result"
os.makedirs(RESULT_DIR, exist_ok=True)

# 모델 & scaler 로드
model  = keras.models.load_model("model/motor_model.h5")
scaler = pickle.load(open("model/scaler.pkl", "rb"))

def run_inference(filepath):
    feats, _, positions = file_to_windows(filepath)

    X = pd.DataFrame(feats).values.astype(np.float32)
    X = scaler.transform(X)

    score = model.predict(X).flatten()
    pred  = (score > THRESHOLD).astype(int)

    normal_cnt = (pred == 0).sum()
    defect_cnt = (pred == 1).sum()
    total      = len(pred)

    print(f"\n[INFER] {os.path.basename(filepath)}")
    print(f"  윈도우 수: {total}")
    print(f"  정상: {normal_cnt} ({normal_cnt/total*100:.1f}%)")
    print(f"  결함: {defect_cnt} ({defect_cnt/total*100:.1f}%)")
    print(f"  평균 anomaly score: {score.mean():.4f}")

    result_df = pd.DataFrame({
        "window_start":  positions,
        "anomaly_score": score,
        "pred_label":    pred
    })

    filename  = os.path.basename(filepath)
    save_name = filename.replace(".csv", "_result.csv")
    save_path = os.path.join(RESULT_DIR, save_name)
    result_df.to_csv(save_path, index=False)
    print(f"  저장: {save_path}")

for f in INFER_FILES:
    run_inference(f)