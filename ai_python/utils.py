# utils.py
import pandas as pd
import numpy as np

# ===== 확정 설정 =====
WINDOW = 50   # 50샘플 = 5초 (10Hz)
STEP   = 25   # 25샘플 오버랩

def extract_features(window):
    az  = window['az']
    cur = window['current']
    vol = window['voltage']
    pwr = window['power']

    return {
        'current_mean': cur.mean(),
        'current_std':  cur.std(),
        'current_p2p':  cur.max() - cur.min(),   # ← 추가
        'voltage_mean': vol.mean(),
        'voltage_std':  vol.std(),                # ← 추가
        'power_mean':   pwr.mean(),
        'power_std':    pwr.std(),                # ← 추가
        'az_p2p':       az.max() - az.min(),      # ← gz_p2p 대체
    }

def file_to_windows(filepath, label=None):
    df = pd.read_csv(filepath)

    if 'label' in df.columns:
        df = df.drop(columns=['label'])
    if 'time_s' in df.columns:
        df = df.drop(columns=['time_s'])

    features, labels, positions = [], [], []

    for start in range(0, len(df) - WINDOW + 1, STEP):
        w = df.iloc[start:start + WINDOW]
        features.append(extract_features(w))
        positions.append(start)

        if label is not None:
            labels.append(label)

    return features, labels, positions