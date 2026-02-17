# utils.py
import pandas as pd
import numpy as np

WINDOW = 50
STEP = 25

def extract_features(window):
    ax, ay, az = window['ax'], window['ay'], window['az']
    gx, gy, gz = window['gx'], window['gy'], window['gz']
    cur = window['current']
    vol = window['voltage']
    pwr = window['power']

    accel_mag = np.sqrt(ax**2 + ay**2 + az**2)

    return {
        'ax_std': ax.std(),
        'ay_std': ay.std(),
        'az_std': az.std(),
        'ax_p2p': ax.max()-ax.min(),
        'ay_p2p': ay.max()-ay.min(),
        'az_p2p': az.max()-az.min(),
        'accel_mag_std': accel_mag.std(),
        'accel_mag_mean': accel_mag.mean(),
        'gx_std': gx.std(),
        'gy_std': gy.std(),
        'gz_std': gz.std(),
        'gx_p2p': gx.max()-gx.min(),
        'gy_p2p': gy.max()-gy.min(),
        'gz_p2p': gz.max()-gz.min(),
        'current_mean': cur.mean(),
        'current_std': cur.std(),
        'current_p2p': cur.max()-cur.min(),
        'voltage_mean': vol.mean(),
        'voltage_std': vol.std(),
        'power_mean': pwr.mean(),
        'current_over_019': (cur >= 0.19).sum()/len(cur),
    }

def file_to_windows(filepath, label=None):
    df = pd.read_csv(filepath)

    features, labels, positions = [], [], []

    for start in range(0, len(df)-WINDOW, STEP):
        w = df.iloc[start:start+WINDOW]
        features.append(extract_features(w))
        positions.append(start)

        if label is not None:
            labels.append(label)

    return features, labels, positions
